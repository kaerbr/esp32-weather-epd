/* Concrete implementation of WeatherProvider for the Bright Sky API.
 *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/                 

#include "config.h"
#ifdef USE_PROVIDER_BRIGHTSKY

#include "provider/BrightSkyProvider.h"
#include "_locale.h"
#include "conversions.h"
#include "display_utils.h" // For getHttpResponsePhrase
#include "SunriseSunset.h"
#include <HTTPClient.h>
#include <time.h>


static const String BRIGHTSKY_ENDPOINT = "api.brightsky.dev";

#ifdef USE_HTTP
  static const uint16_t PORT = 80;
#else
  static const uint16_t PORT = 443;
#endif

// Helper to keep track of hourly index across two API calls
static int hourly_idx = 0;

BrightSkyProvider::BrightSkyProvider(WiFiClient& client) : wifi_client(client) {
    hourly_idx = 0; // Reset for new instance
}

BrightSkyProvider::~BrightSkyProvider() {
}

bool BrightSkyProvider::fetchWeatherData(WeatherData& data) {
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);

    char today_str[11];
    strftime(today_str, sizeof(today_str), "%Y-%m-%d", timeinfo);

    time_t tomorrow = now + 86400;
    struct tm* tomorrow_timeinfo = localtime(&tomorrow);
    char tomorrow_str[11];
    strftime(tomorrow_str, sizeof(tomorrow_str), "%Y-%m-%d", tomorrow_timeinfo);

    JsonDocument doc;

    // Fetch today's data
    int http_code_today = fetchForecastData(doc, today_str);
    if (http_code_today != HTTP_CODE_OK) {
        Serial.println("Failed to get Bright Sky data for today.");
        lastHttpResponseCode = http_code_today;
        return false;
    }
    deserializeForecast(doc, data);

    // Fetch tomorrow's data
    int http_code_tomorrow = fetchForecastData(doc, tomorrow_str);
    if (http_code_tomorrow == HTTP_CODE_OK) {
        deserializeForecast(doc, data);
    }

    if (hourly_idx == 0) {
        Serial.println("No hourly data was deserialized.");
        return false;
    }

    // --- Populate Current Weather ---
    time(&now);
    long closest_time_diff = -1;
    int current_weather_idx = 0;

    for (int i = 0; i < hourly_idx; ++i) {
        long time_diff = abs(data.hourly[i].dt - now);
        if (closest_time_diff == -1 || time_diff < closest_time_diff) {
            closest_time_diff = time_diff;
            current_weather_idx = i;
        }
    }
    
    // Copy data from the closest hourly forecast to current
    data.current.dt = data.hourly[current_weather_idx].dt;
    data.current.temp = data.hourly[current_weather_idx].temp;
    data.current.humidity = 0; // Not in hourly data
    data.current.wind_speed = data.hourly[current_weather_idx].wind_speed;
    data.current.wind_gust = data.hourly[current_weather_idx].wind_gust;
    data.current.wind_deg = 0; // Not in hourly data
    data.current.clouds = data.hourly[current_weather_idx].clouds;
    data.current.weather = data.hourly[current_weather_idx].weather;
    // Missing from API: feels_like, pressure, uvi, visibility, sunrise, sunset
    data.current.feels_like = data.current.temp; // Best guess
    data.current.pressure = 0;
    data.current.uvi = 0;
    data.current.visibility = 0;


    // --- Calculate Daily Summaries ---
    int daily_idx = -1;
    int current_day = -1;

    for (int i = 0; i < hourly_idx; ++i) {
        time_t hourly_time = data.hourly[i].dt;
        struct tm* hourly_tm = localtime(&hourly_time);
        int day_of_year = hourly_tm->tm_yday;

        if (day_of_year != current_day) {
            daily_idx++;
            if (daily_idx >= MAX_DAILY_FORECASTS) break;
            
            current_day = day_of_year;
            data.daily[daily_idx].dt = data.hourly[i].dt;
            data.daily[daily_idx].temp.min = data.hourly[i].temp;
            data.daily[daily_idx].temp.max = data.hourly[i].temp;
            data.daily[daily_idx].weather = data.hourly[i].weather; // Use first hour's weather as representative

            // Calculate sunrise/sunset for this day
            time_t sunrise_t, sunset_t;
            calculateSunriseSunset(hourly_tm->tm_year + 1900, hourly_tm->tm_mon + 1, hourly_tm->tm_mday, 
                                   atof(LAT.c_str()), atof(LON.c_str()), 
                                   &sunrise_t, &sunset_t);
            data.daily[daily_idx].sunrise = sunrise_t;
            data.daily[daily_idx].sunset = sunset_t;
        } else {
            if (data.hourly[i].temp < data.daily[daily_idx].temp.min) {
                data.daily[daily_idx].temp.min = data.hourly[i].temp;
            }
            if (data.hourly[i].temp > data.daily[daily_idx].temp.max) {
                data.daily[daily_idx].temp.max = data.hourly[i].temp;
            }
        }
    }
    
    // Set current sunrise/sunset from today's daily forecast
    if (daily_idx >= 0) {
        data.current.sunrise = data.daily[0].sunrise;
        data.current.sunset = data.daily[0].sunset;
    }

    convertUnits(data);

    return true;
}

int BrightSkyProvider::fetchForecastData(JsonDocument& doc, const String& date) {
    int attempts = 0;
    bool rxSuccess = false;
    
    String uri = "/weather?lat=" + LAT + "&lon=" + LON + "&date=" + date;
    
    Serial.print(TXT_ATTEMPTING_HTTP_REQ);
    Serial.println(": " + BRIGHTSKY_ENDPOINT + uri);
    int httpResponse = 0;

    while (!rxSuccess && attempts < 3) {
        if (WiFi.status() != WL_CONNECTED) {
            return -512 - static_cast<int>(WiFi.status());
        }

        HTTPClient http;
        http.setConnectTimeout(HTTP_CLIENT_TCP_TIMEOUT);
        http.setTimeout(HTTP_CLIENT_TCP_TIMEOUT);

        http.begin(wifi_client, BRIGHTSKY_ENDPOINT, PORT, uri);

        httpResponse = http.GET();
        if (httpResponse == HTTP_CODE_OK) {
            DeserializationError error = deserializeJson(doc, http.getStream());
            if (error) {
                Serial.println("JSON Deserialization failed: " + String(error.c_str()));
                httpResponse = -256 - static_cast<int>(error.code());
            }
            rxSuccess = !error;
        }
        
        http.end();
        Serial.println("  " + String(httpResponse, DEC) + " " + getHttpResponsePhrase(httpResponse));
        attempts++;
    }

    return httpResponse;
}

void BrightSkyProvider::deserializeForecast(JsonDocument& doc, WeatherData& data) {
    JsonArray weather = doc["weather"].as<JsonArray>();
    for (JsonObject hourly : weather) {
        if (hourly_idx >= MAX_HOURLY_FORECASTS) break;

        const char* timestamp_str = hourly["timestamp"];
        struct tm tm = {0};
        strptime(timestamp_str, "%Y-%m-%dT%H:%M:%S", &tm);
        time_t timestamp = mktime(&tm);

        data.hourly[hourly_idx].dt = timestamp;
        data.hourly[hourly_idx].temp = hourly["temperature"].as<float>();
        data.hourly[hourly_idx].pop = hourly["precipitation_probability"].as<float>() / 100.0;
        data.hourly[hourly_idx].rain_1h = hourly["precipitation"].as<float>();
        data.hourly[hourly_idx].snow_1h = 0; // Not provided
        data.hourly[hourly_idx].clouds = hourly["cloud_cover"].as<int>();
        data.hourly[hourly_idx].wind_speed = hourly["wind_speed"].as<float>();
        data.hourly[hourly_idx].wind_gust = hourly["wind_gust_speed"].as<float>();
        
        data.hourly[hourly_idx].weather.icon = hourly["icon"].as<const char*>();
        data.hourly[hourly_idx].weather.main = hourly["condition"].as<const char*>();
        data.hourly[hourly_idx].weather.id = 0; // Not provided
        
        hourly_idx++;
    }
}

void BrightSkyProvider::convertUnits(WeatherData& data) {
    // First, convert provider's base units (km/h) to our project's base units (m/s)
    data.current.wind_speed = kilometersperhour_to_meterspersecond(data.current.wind_speed);
    data.current.wind_gust = kilometersperhour_to_meterspersecond(data.current.wind_gust);
    for (int i = 0; i < hourly_idx; ++i) {
        data.hourly[i].wind_speed = kilometersperhour_to_meterspersecond(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = kilometersperhour_to_meterspersecond(data.hourly[i].wind_gust);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i) {
        if (data.daily[i].dt == 0) break;
        data.daily[i].wind_speed = kilometersperhour_to_meterspersecond(data.daily[i].wind_speed);
        data.daily[i].wind_gust = kilometersperhour_to_meterspersecond(data.daily[i].wind_gust);
    }

    // Now, convert from our base units to the user's configured units
#if defined(UNITS_TEMP_FAHRENHEIT)
    data.current.temp = celsius_to_fahrenheit(data.current.temp);
    data.current.feels_like = celsius_to_fahrenheit(data.current.feels_like);
    for (int i = 0; i < hourly_idx; ++i) {
        data.hourly[i].temp = celsius_to_fahrenheit(data.hourly[i].temp);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i) {
        if (data.daily[i].dt == 0) break;
        data.daily[i].temp.min = celsius_to_fahrenheit(data.daily[i].temp.min);
        data.daily[i].temp.max = celsius_to_fahrenheit(data.daily[i].temp.max);
    }
#elif defined(UNITS_TEMP_KELVIN)
    data.current.temp = celsius_to_kelvin(data.current.temp);
    data.current.feels_like = celsius_to_kelvin(data.current.feels_like);
    for (int i = 0; i < hourly_idx; ++i) {
        data.hourly[i].temp = celsius_to_kelvin(data.hourly[i].temp);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i) {
        if (data.daily[i].dt == 0) break;
        data.daily[i].temp.min = celsius_to_kelvin(data.daily[i].temp.min);
        data.daily[i].temp.max = celsius_to_kelvin(data.daily[i].temp.max);
    }
#endif

#if defined(UNITS_SPEED_FEETPERSECOND)
    data.current.wind_speed = meterspersecond_to_feetpersecond(data.current.wind_speed);
    data.current.wind_gust = meterspersecond_to_feetpersecond(data.current.wind_gust);
    for (int i = 0; i < hourly_idx; ++i) {
        data.hourly[i].wind_speed = meterspersecond_to_feetpersecond(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = meterspersecond_to_feetpersecond(data.hourly[i].wind_gust);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i) {
        if (data.daily[i].dt == 0) break;
        data.daily[i].wind_speed = meterspersecond_to_feetpersecond(data.daily[i].wind_speed);
        data.daily[i].wind_gust = meterspersecond_to_feetpersecond(data.daily[i].wind_gust);
    }
#elif defined(UNITS_SPEED_KILOMETERSPERHOUR)
    // Already in km/h, but our base is m/s, so we convert back
    data.current.wind_speed = meterspersecond_to_kilometersperhour(data.current.wind_speed);
    data.current.wind_gust = meterspersecond_to_kilometersperhour(data.current.wind_gust);
    for (int i = 0; i < hourly_idx; ++i) {
        data.hourly[i].wind_speed = meterspersecond_to_kilometersperhour(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = meterspersecond_to_kilometersperhour(data.hourly[i].wind_gust);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i) {
        if (data.daily[i].dt == 0) break;
        data.daily[i].wind_speed = meterspersecond_to_kilometersperhour(data.daily[i].wind_speed);
        data.daily[i].wind_gust = meterspersecond_to_kilometersperhour(data.daily[i].wind_gust);
    }
#elif defined(UNITS_SPEED_MILESPERHOUR)
    data.current.wind_speed = meterspersecond_to_milesperhour(data.current.wind_speed);
    data.current.wind_gust = meterspersecond_to_milesperhour(data.current.wind_gust);
    for (int i = 0; i < hourly_idx; ++i) {
        data.hourly[i].wind_speed = meterspersecond_to_milesperhour(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = meterspersecond_to_milesperhour(data.hourly[i].wind_gust);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i) {
        if (data.daily[i].dt == .0) break;
        data.daily[i].wind_speed = meterspersecond_to_milesperhour(data.daily[i].wind_speed);
        data.daily[i].wind_gust = meterspersecond_to_milesperhour(data.daily[i].wind_gust);
    }
#elif defined(UNITS_SPEED_KNOTS)
    data.current.wind_speed = meterspersecond_to_knots(data.current.wind_speed);
    data.current.wind_gust = meterspersecond_to_knots(data.current.wind_gust);
    for (int i = 0; i < hourly_idx; ++i) {
        data.hourly[i].wind_speed = meterspersecond_to_knots(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = meterspersecond_to_knots(data.hourly[i].wind_gust);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i) {
        if (data.daily[i].dt == 0) break;
        data.daily[i].wind_speed = meterspersecond_to_knots(data.daily[i].wind_speed);
        data.daily[i].wind_gust = meterspersecond_to_knots(data.daily[i].wind_gust);
    }
#elif defined(UNITS_SPEED_BEAUFORT)
    data.current.wind_speed = meterspersecond_to_beaufort(data.current.wind_speed);
    data.current.wind_gust = meterspersecond_to_beaufort(data.current.wind_gust);
    for (int i = 0; i < hourly_idx; ++i) {
        data.hourly[i].wind_speed = meterspersecond_to_beaufort(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = meterspersecond_to_beaufort(data.hourly[i].wind_gust);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i) {
        if (data.daily[i].dt == 0) break;
        data.daily[i].wind_speed = meterspersecond_to_beaufort(data.daily[i].wind_speed);
        data.daily[i].wind_gust = meterspersecond_to_beaufort(data.daily[i].wind_gust);
    }
#endif
}

#endif