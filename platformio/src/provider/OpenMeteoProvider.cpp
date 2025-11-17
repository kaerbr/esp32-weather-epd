/* Concrete implementation of WeatherProvider for the Open-Meteo API.
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
#ifdef USE_PROVIDER_OPENMETEO

#include "provider/OpenMeteoProvider.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>

OpenMeteoProvider::OpenMeteoProvider(WiFiClient &client) : wifi_client(client)
{
    providerName = "Open-Meteo";
}

OpenMeteoProvider::~OpenMeteoProvider()
{
    // The client is owned by the caller, so the destructor is empty.
}

bool OpenMeteoProvider::fetchWeatherData(WeatherData &data)
{
    HTTPClient http;
    String server = "api.open-meteo.com";

    String temp_unit;
#if defined(UNITS_TEMP_CELSIUS)
    temp_unit = "celsius";
#elif defined(UNITS_TEMP_FAHRENHEIT)
    temp_unit = "fahrenheit";
#else
    // Default to celsius if no specific temperature unit is defined.
    // The config validation in config.h should prevent this case.
    temp_unit = "celsius";
#endif

    String wind_unit;
#if defined(UNITS_SPEED_KILOMETERSPERHOUR)
    wind_unit = "kmh";
#elif defined(UNITS_SPEED_MILESPERHOUR)
    wind_unit = "mph";
#elif defined(UNITS_SPEED_METERSPERSECOND)
    wind_unit = "ms";
#elif defined(UNITS_SPEED_KNOTS)
    wind_unit = "kn";
#else
    // Default to m/s if no specific wind speed unit is defined.
    // The config validation in config.h should prevent this case.
    wind_unit = "ms";
#endif

    String url = "/v1/forecast?latitude=" + LAT +
                 "&longitude=" + LON +
                 "&current=temperature_2m,apparent_temperature,relativehumidity_2m,surface_pressure,windspeed_10m,winddirection_10m,windgusts_10m,weathercode,visibility" +
                 "&hourly=weathercode,temperature_2m,precipitation_probability,rain,snowfall,cloudcover,windspeed_10m,windgusts_10m" +
                 "&daily=weathercode,temperature_2m_max,temperature_2m_min,sunrise,sunset,uv_index_max,rain_sum,snowfall_sum,precipitation_probability_max,windspeed_10m_max,windgusts_10m_max" +
                 "&temperature_unit=" + temp_unit +
                 "&windspeed_unit=" + wind_unit +
                 "&forecast_days=" + MAX_DAILY_FORECASTS +
                 "&timeformat=unixtime&timezone=auto";
    Serial.println(url);

    if (!http.begin(wifi_client, server, 443, url, true))
    {
        log_e("HTTP connection failed");
        return false;
    }

    int httpCode = http.GET();
    if (httpCode <= 0 || httpCode != HTTP_CODE_OK)
    {
        log_e("HTTP GET failed, code: %d", httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        log_e("deserializeJson() failed: %s", error.c_str());
        return false;
    }

    JsonObject current = doc["current"];
    if (current.isNull())
    {
        log_e("JSON parsing error: 'current' object not found");
        return false;
    }

    JsonObject daily = doc["daily"];
    if (daily.isNull())
    {
        log_e("JSON parsing error: 'daily' object not found");
        return false;
    }

    // Fill data.current with real data from the API
    data.current.dt = current["time"];
    data.current.sunrise = daily["sunrise"][0];
    data.current.sunset = daily["sunset"][0];
    data.current.temp = current["temperature_2m"];
    data.current.feels_like = current["apparent_temperature"];
    data.current.humidity = current["relativehumidity_2m"];
    data.current.wind_deg = current["winddirection_10m"];
    data.current.wind_speed = current["windspeed_10m"];
    data.current.wind_gust = current["windgusts_10m"];
    data.current.uvi = daily["uv_index_max"][0];
    data.current.pressure = current["surface_pressure"];
#if defined(UNITS_DIST_KILOMETERS)
    data.current.visibility = current["visibility"].as<float>() / 1000.0f; // meters to kilometers
#elif defined(UNITS_DIST_MILES)
    data.current.visibility = current["visibility"].as<float>() / 1609.34f; // meters to miles
#else
    data.current.visibility = current["visibility"]; // Default to meters if no unit defined
#endif
    data.current.weather.id = current["weathercode"];
    data.current.weather.main = "";      // Open-Meteo does not provide this
    data.current.weather.description = ""; // Open-Meteo does not provide this
    data.current.weather.icon = "";         // Open-Meteo does not provide this

    // Fill data.daily with real data from the API
    JsonArray daily_time = daily["time"];
    if (daily_time.size() < MAX_DAILY_FORECASTS)
    {
        log_e("JSON parsing error: 'daily' object does not contain enough forecast days");
        return false;
    }

    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i)
    {
      data.daily[i].dt = daily["time"][i];
      data.daily[i].sunrise = daily["sunrise"][i];
      data.daily[i].sunset = daily["sunset"][i];
      data.daily[i].moonrise = 0; // Default to 0 as no real data from API
      data.daily[i].moonset = 0;   // Default to 0 as no real data from API
      data.daily[i].moon_phase = 0.0f; // Default to 0.0f as no real data from API
      data.daily[i].temp.min = daily["temperature_2m_min"][i];
      data.daily[i].temp.max = daily["temperature_2m_max"][i];
      data.daily[i].pop = daily["precipitation_probability_max"][i].as<float>() / 100.0f;
      data.daily[i].rain = daily["rain_sum"][i];
      data.daily[i].snow = daily["snowfall_sum"][i];
      data.daily[i].clouds = 0; // Not provided by Open-Meteo daily forecast
      data.daily[i].wind_speed = daily["windspeed_10m_max"][i];
      data.daily[i].wind_gust = daily["windgusts_10m_max"][i];
      data.daily[i].weather = {daily["weathercode"][i], "", "", ""};
    }

    JsonObject hourly = doc["hourly"];
    if (hourly.isNull())
    {
        log_e("JSON parsing error: 'hourly' object not found");
        return false;
    }

    JsonArray hourly_time = hourly["time"];

    // Find the starting index for the hourly forecast.
    // Find the last hourly timestamp that is less than or equal to the current time.
    int startIndex = -1;
    for (int i = 0; i < hourly_time.size(); i++) {
        if (hourly_time[i].as<time_t>() <= data.current.dt) {
            startIndex = i;
        } else {
            // The hourly_time array is sorted, so we can stop once we pass the current time.
            break;
        }
    }

    if (startIndex == -1) {
        // This can happen if the current time is before the first forecast hour.
        // As a fallback, log a warning and start from the beginning of the forecast.
        log_w("Could not find a past hourly forecast slot; starting from the first available hour.");
        startIndex = 0;
    }

    // Check if there are enough forecast hours from the start index
    if (hourly_time.size() < startIndex + MAX_HOURLY_FORECASTS)
    {
        log_e("JSON parsing error: 'hourly' object does not contain enough forecast hours from the current time");
        return false;
    }

    // Fill data.hourly with real data from the API, starting from the current hour's slot
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i)
    {
      int dataIndex = startIndex + i;
      data.hourly[i].dt = hourly["time"][dataIndex];
      data.hourly[i].temp = hourly["temperature_2m"][dataIndex];
      data.hourly[i].pop = hourly["precipitation_probability"][dataIndex].as<float>() / 100.0f;
      data.hourly[i].rain_1h = hourly["rain"][dataIndex];
      data.hourly[i].snow_1h = hourly["snowfall"][dataIndex];
      data.hourly[i].clouds = hourly["cloudcover"][dataIndex];
      data.hourly[i].wind_speed = hourly["windspeed_10m"][dataIndex];
      data.hourly[i].wind_gust = hourly["windgusts_10m"][dataIndex];
      data.hourly[i].weather = {hourly["weathercode"][dataIndex], "", "", ""};
    }

    // Dummy WeatherAlerts
    //data.alerts.clear();
    //data.alerts.push_back({"Heat Advisory", "Extreme Temperature", 1678886400, 1678972800});

    // Fetch and fill AirQuality data
    { // Use a new scope for the second HTTP request
        HTTPClient aq_http;
        String aq_server = "air-quality-api.open-meteo.com";
        String aq_url = "/v1/air-quality?latitude=" + LAT +
                        "&longitude=" + LON +
                        "&current=european_aqi" +
                        "&timeformat=unixtime&timezone=auto";

        if (!aq_http.begin(wifi_client, aq_server, 443, aq_url, true))
        {
            log_w("Air quality HTTP connection failed, skipping.");
            data.air_quality.aqi = 0; // Set to a default "unknown" value
        }
        else
        {
            int aq_httpCode = aq_http.GET();
            if (aq_httpCode > 0 && aq_httpCode == HTTP_CODE_OK)
            {
                String aq_payload = aq_http.getString();
                aq_http.end();

                doc.clear(); // Clear the document before reusing
                DeserializationError aq_error = deserializeJson(doc, aq_payload);

                if (aq_error)
                {
                    log_w("Air quality JSON parsing failed: %s", aq_error.c_str());
                    data.air_quality.aqi = 0;
                }
                else
                {
                    // Use | 0 as a default if the key is missing from the response
                    data.air_quality.aqi = doc["current"]["european_aqi"] | 0;
                }
            }
            else
            {
                log_w("Air quality HTTP GET failed, code: %d", aq_httpCode);
                data.air_quality.aqi = 0;
                aq_http.end();
            }
        }
    }

    // Fill Metadata from API response
    data.lat = doc["latitude"];
    data.lon = doc["longitude"];
    data.timezone = doc["timezone"].as<String>();
    data.timezone_offset = doc["utc_offset_seconds"];

    

    Serial.println("--- WeatherData ---");
    Serial.printf("Lat: %f, Lon: %f\n", data.lat, data.lon);
    Serial.printf("Timezone: %s, Offset: %d\n", data.timezone.c_str(), data.timezone_offset);

    Serial.println("-- Current --");
    Serial.printf("  dt: %lld\n", data.current.dt);
    Serial.printf("  sunrise: %lld\n", data.current.sunrise);
    Serial.printf("  sunset: %lld\n", data.current.sunset);
    Serial.printf("  temp: %f\n", data.current.temp);
    Serial.printf("  feels_like: %f\n", data.current.feels_like);
    Serial.printf("  pressure: %d\n", data.current.pressure);
    Serial.printf("  humidity: %d\n", data.current.humidity);
    Serial.printf("  uvi: %f\n", data.current.uvi);
    Serial.printf("  visibility: %d\n", data.current.visibility);
    Serial.printf("  wind_speed: %f\n", data.current.wind_speed);
    Serial.printf("  wind_gust: %f\n", data.current.wind_gust);
    Serial.printf("  wind_deg: %d\n", data.current.wind_deg);
    Serial.printf("  clouds: %d\n", data.current.clouds);
    Serial.printf("  weather id: %d\n", data.current.weather.id);
    Serial.printf("  weather main: %s\n", data.current.weather.main.c_str());
    Serial.printf("  weather description: %s\n", data.current.weather.description.c_str());
    Serial.printf("  weather icon: %s\n", data.current.weather.icon.c_str());

    Serial.println("-- Hourly --");
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i) {
        Serial.printf("  -- Hour %d --\n", i);
        Serial.printf("    dt: %lld\n", data.hourly[i].dt);
        Serial.printf("    temp: %f\n", data.hourly[i].temp);
        Serial.printf("    pop: %f\n", data.hourly[i].pop);
        Serial.printf("    rain_1h: %f\n", data.hourly[i].rain_1h);
        Serial.printf("    snow_1h: %f\n", data.hourly[i].snow_1h);
        Serial.printf("    clouds: %d\n", data.hourly[i].clouds);
        Serial.printf("    wind_speed: %f\n", data.hourly[i].wind_speed);
        Serial.printf("    wind_gust: %f\n", data.hourly[i].wind_gust);
        Serial.printf("    weather id: %d\n", data.hourly[i].weather.id);
        Serial.printf("    weather icon: %s\n", data.hourly[i].weather.icon.c_str());
    }

    Serial.println("-- Daily --");
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i) {
        Serial.printf("  -- Day %d --\n", i);
        Serial.printf("    dt: %lld\n", data.daily[i].dt);
        Serial.printf("    sunrise: %lld\n", data.daily[i].sunrise);
        Serial.printf("    sunset: %lld\n", data.daily[i].sunset);
        Serial.printf("    moonrise: %lld\n", data.daily[i].moonrise);
        Serial.printf("    moonset: %lld\n", data.daily[i].moonset);
        Serial.printf("    moon_phase: %f\n", data.daily[i].moon_phase);
        Serial.printf("    temp min: %f\n", data.daily[i].temp.min);
        Serial.printf("    temp max: %f\n", data.daily[i].temp.max);
        Serial.printf("    pop: %f\n", data.daily[i].pop);
        Serial.printf("    rain: %f\n", data.daily[i].rain);
        Serial.printf("    snow: %f\n", data.daily[i].snow);
        Serial.printf("    clouds: %d\n", data.daily[i].clouds);
        Serial.printf("    wind_speed: %f\n", data.daily[i].wind_speed);
        Serial.printf("    wind_gust: %f\n", data.daily[i].wind_gust);
        Serial.printf("    weather id: %d\n", data.daily[i].weather.id);
        Serial.printf("    weather icon: %s\n", data.daily[i].weather.icon.c_str());
    }

    Serial.println("-- Air Quality --");
    Serial.printf("  aqi: %d\n", data.air_quality.aqi);

    Serial.println("-- Alerts --");
    for (const auto& alert : data.alerts) {
        Serial.printf("  event: %s\n", alert.event.c_str());
        Serial.printf("  start: %lld\n", alert.start);
        Serial.printf("  end: %lld\n", alert.end);
        Serial.printf("  tags: %s\n", alert.tags.c_str());
    }
    Serial.println("-------------------");




    return true;
}

#endif