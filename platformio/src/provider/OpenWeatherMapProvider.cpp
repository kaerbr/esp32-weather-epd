/* Concrete implementation of WeatherProvider for the OpenWeatherMap API.
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
#ifdef USE_PROVIDER_OPENWEATHERMAP

#include "provider/OpenWeatherMapProvider.h"
#include "_locale.h"
#include "config.h"
#include "conversions.h"
#include "display_utils.h" // For getHttpResponsePhrase
#include "aqi.h"

#include <HTTPClient.h>

static const int AIR_POLLUTION_HISTORY_HOURS = 24;
static const String API_ENDPOINT = "api.openweathermap.org";

OpenWeatherMapProvider::OpenWeatherMapProvider(WiFiClient &client) : wifi_client(client)
{
    providerName = "OpenWeatherMap";
}

OpenWeatherMapProvider::~OpenWeatherMapProvider()
{
    // The client is owned by the caller, so the destructor is empty.
}

int OpenWeatherMapProvider::fetchWeatherData(WeatherData &data)
{
    int onecall_http_code = fetchOneCallData(data);
    if (onecall_http_code != HTTP_CODE_OK)
    {
        Serial.println("Failed to get OneCall data.");
        return onecall_http_code;
    }

    int air_http_code = fetchAirPollutionData(data);
    if (air_http_code != HTTP_CODE_OK)
    {
        Serial.println("Failed to get Air Pollution data.");
        return air_http_code;
    }

    convertUnits(data);

    return HTTP_CODE_OK;
}

int OpenWeatherMapProvider::fetchOneCallData(WeatherData &data)
{
    int attempts = 0;
    bool rxSuccess = false;
    DeserializationError jsonErr;
    String units = "metric";
    String uri = "/data/3.0/onecall?lat=" + LAT + "&lon=" + LON + "&lang=" + LANGUAGE + "&units=" + units + "&exclude=minutely";
#if DISPLAY_ALERTS
    uri += ",alerts";
#endif

    String sanitizedUri = API_ENDPOINT + uri + "&appid={API key}";
    uri += "&appid=" + APIKEY;

    Serial.print(TXT_ATTEMPTING_HTTP_REQ);
    Serial.println(": " + sanitizedUri);
    int httpResponse = 0;

    while (!rxSuccess && attempts < 3)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            return -512 - static_cast<int>(WiFi.status());
        }

        HTTPClient http;
        http.setConnectTimeout(HTTP_CLIENT_TCP_TIMEOUT);
        http.setTimeout(HTTP_CLIENT_TCP_TIMEOUT);

        http.begin(wifi_client, API_ENDPOINT, PORT, uri);

        httpResponse = http.GET();
        if (httpResponse == HTTP_CODE_OK)
        {
            jsonErr = deserializeOneCall(http.getStream(), data);
            if (jsonErr)
            {
                httpResponse = -256 - static_cast<int>(jsonErr.code());
            }
            rxSuccess = !jsonErr;
        }

        http.end();
        Serial.println("  " + String(httpResponse, DEC) + " " + getHttpResponsePhrase(httpResponse));
        ++attempts;
    }

    return httpResponse;
}

int OpenWeatherMapProvider::fetchAirPollutionData(WeatherData &data)
{
    int attempts = 0;
    bool rxSuccess = false;
    DeserializationError jsonErr;

    time_t now;
    int64_t end = time(&now);
    int64_t start = end - ((3600 * AIR_POLLUTION_HISTORY_HOURS) - 1); // 24 hours of data
    char endStr[22];
    char startStr[22];
    sprintf(endStr, "%lld", end);
    sprintf(startStr, "%lld", start);
    String uri = "/data/2.5/air_pollution/history?lat=" + LAT + "&lon=" + LON + "&start=" + startStr + "&end=" + endStr + "&appid=" + APIKEY;

    String sanitizedUri = API_ENDPOINT + uri;

    Serial.print(TXT_ATTEMPTING_HTTP_REQ);
    Serial.println(": " + sanitizedUri);
    int httpResponse = 0;

    while (!rxSuccess && attempts < 3)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            return -512 - static_cast<int>(WiFi.status());
        }

        HTTPClient http;
        http.setConnectTimeout(HTTP_CLIENT_TCP_TIMEOUT);
        http.setTimeout(HTTP_CLIENT_TCP_TIMEOUT);

        http.begin(wifi_client, API_ENDPOINT, PORT, uri);

        httpResponse = http.GET();
        if (httpResponse == HTTP_CODE_OK)
        {
            jsonErr = deserializeAirQuality(http.getStream(), data);
            if (jsonErr)
            {
                httpResponse = -256 - static_cast<int>(jsonErr.code());
            }
            rxSuccess = !jsonErr;
        }

        http.end();
        Serial.println("  " + String(httpResponse, DEC) + " " + getHttpResponsePhrase(httpResponse));
        ++attempts;
    }

    return httpResponse;
}

DeserializationError OpenWeatherMapProvider::deserializeOneCall(WiFiClient &json, WeatherData &data)
{
    JsonDocument filter;
    filter["current"] = true;
    filter["minutely"] = false;
    filter["hourly"] = true;
    filter["daily"] = true;
#if !DISPLAY_ALERTS
    filter["alerts"] = false;
#else
    // description can be very long so they are filtered out to save on memory
    // along with sender_name
    for (int i = 0; i < MAX_ALERTS; ++i)
    {
        filter["alerts"][i]["sender_name"] = false;
        filter["alerts"][i]["event"] = true;
        filter["alerts"][i]["start"] = true;
        filter["alerts"][i]["end"] = true;
        filter["alerts"][i]["description"] = false;
        filter["alerts"][i]["tags"] = true;
    }
#endif

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, json, DeserializationOption::Filter(filter));
#if DEBUG_LEVEL >= 1
    Serial.println("[debug] doc.overflowed() : " + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
    serializeJsonPretty(doc, Serial);
#endif
    if (error)
    {
        return error;
    }

    data.lat = doc["lat"].as<float>();
    data.lon = doc["lon"].as<float>();
    data.timezone = doc["timezone"].as<const char *>();
    data.timezone_offset = doc["timezone_offset"].as<int>();

    JsonObject current = doc["current"];
    data.current.dt = current["dt"].as<int64_t>();
    data.current.sunrise = current["sunrise"].as<int64_t>();
    data.current.sunset = current["sunset"].as<int64_t>();
    data.current.temp = current["temp"].as<float>();
    data.current.feels_like = current["feels_like"].as<float>();
    data.current.pressure = current["pressure"].as<int>();
    data.current.humidity = current["humidity"].as<int>();
    data.current.uvi = current["uvi"].as<float>();
    data.current.visibility = current["visibility"].as<int>();
    data.current.wind_speed = current["wind_speed"].as<float>();
    data.current.wind_gust = current["wind_gust"].as<float>();
    data.current.wind_deg = current["wind_deg"].as<int>();
    data.current.clouds = current["clouds"].as<int>();
    JsonObject current_weather = current["weather"][0];
    data.current.weather.id = current_weather["id"].as<int>();
    data.current.weather.main = current_weather["main"].as<const char *>();
    data.current.weather.description = current_weather["description"].as<const char *>();
    data.current.weather.icon = current_weather["icon"].as<const char *>();

    int i = 0;
    for (JsonObject hourly : doc["hourly"].as<JsonArray>())
    {
        if (i >= MAX_HOURLY_FORECASTS)
            break;
        data.hourly[i].dt = hourly["dt"].as<int64_t>();
        data.hourly[i].temp = hourly["temp"].as<float>();
        data.hourly[i].pop = hourly["pop"].as<float>();
        data.hourly[i].rain_1h = hourly["rain"]["1h"].as<float>();
        data.hourly[i].snow_1h = hourly["snow"]["1h"].as<float>();
        data.hourly[i].clouds = hourly["clouds"].as<int>();
        data.hourly[i].wind_speed = hourly["wind_speed"].as<float>();
        data.hourly[i].wind_gust = hourly["wind_gust"].as<float>();
        JsonObject hourly_weather = hourly["weather"][0];
        data.hourly[i].weather.id = hourly_weather["id"].as<int>();
        data.hourly[i].weather.icon = hourly_weather["icon"].as<const char *>();
        i++;
    }

    i = 0;
    for (JsonObject daily : doc["daily"].as<JsonArray>())
    {
        if (i >= MAX_DAILY_FORECASTS)
            break;
        data.daily[i].dt = daily["dt"].as<int64_t>();
        data.daily[i].sunrise = daily["sunrise"].as<int64_t>();
        data.daily[i].sunset = daily["sunset"].as<int64_t>();
        data.daily[i].moonrise = daily["moonrise"].as<int64_t>();
        data.daily[i].moonset = daily["moonset"].as<int64_t>();
        data.daily[i].moon_phase = daily["moon_phase"].as<float>();
        JsonObject daily_temp = daily["temp"];
        data.daily[i].temp.min = daily_temp["min"].as<float>();
        data.daily[i].temp.max = daily_temp["max"].as<float>();
        data.daily[i].pop = daily["pop"].as<float>();
        data.daily[i].rain = daily["rain"].as<float>();
        data.daily[i].snow = daily["snow"].as<float>();
        data.daily[i].clouds = daily["clouds"].as<int>();
        data.daily[i].wind_speed = daily["wind_speed"].as<float>();
        data.daily[i].wind_gust = daily["wind_gust"].as<float>();
        JsonObject daily_weather = daily["weather"][0];
        data.daily[i].weather.id = daily_weather["id"].as<int>();
        data.daily[i].weather.icon = daily_weather["icon"].as<const char *>();
        i++;
    }

#if DISPLAY_ALERTS
    data.alerts.clear();
    for (JsonObject alerts : doc["alerts"].as<JsonArray>())
    {
        WeatherAlert new_alert;
        new_alert.event = alerts["event"].as<const char *>();
        new_alert.start = alerts["start"].as<int64_t>();
        new_alert.end = alerts["end"].as<int64_t>();
        new_alert.tags = alerts["tags"][0].as<const char *>();
        data.alerts.push_back(new_alert);
    }
#endif

    return error;
}

DeserializationError OpenWeatherMapProvider::deserializeAirQuality(WiFiClient &json, WeatherData &data)
{
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, json);
#if DEBUG_LEVEL >= 1
    Serial.println("[debug] doc.overflowed() : " + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
    serializeJsonPretty(doc, Serial);
#endif
    if (error)
    {
        return error;
    }

    float co[AIR_POLLUTION_HISTORY_HOURS] = {0}, nh3[AIR_POLLUTION_HISTORY_HOURS] = {0}, no[AIR_POLLUTION_HISTORY_HOURS] = {0}, no2[AIR_POLLUTION_HISTORY_HOURS] = {0}, o3[AIR_POLLUTION_HISTORY_HOURS] = {0}, so2[AIR_POLLUTION_HISTORY_HOURS] = {0}, pm10[AIR_POLLUTION_HISTORY_HOURS] = {0}, pm2_5[AIR_POLLUTION_HISTORY_HOURS] = {0};
    int i = 0;

    for (JsonObject item : doc["list"].as<JsonArray>())
    {
        if (i >= AIR_POLLUTION_HISTORY_HOURS)
            break;
        JsonObject components = item["components"];
        co[i] = components["co"].as<float>();
        nh3[i] = components["nh3"].as<float>();
        no[i] = components["no"].as<float>();
        no2[i] = components["no2"].as<float>();
        o3[i] = components["o3"].as<float>();
        so2[i] = components["so2"].as<float>();
        pm10[i] = components["pm10"].as<float>();
        pm2_5[i] = components["pm2_5"].as<float>();
        i++;
    }

    data.air_quality.aqi = calc_aqi(AQI_SCALE, co, nh3, no, no2, o3, NULL, so2, pm10, pm2_5);

    return error;
}

void OpenWeatherMapProvider::convertUnits(WeatherData &data)
{
#if defined(UNITS_TEMP_FAHRENHEIT)
    data.current.temp = celsius_to_fahrenheit(data.current.temp);
    data.current.feels_like = celsius_to_fahrenheit(data.current.feels_like);
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i)
    {
        data.hourly[i].temp = celsius_to_fahrenheit(data.hourly[i].temp);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i)
    {
        data.daily[i].temp.min = celsius_to_fahrenheit(data.daily[i].temp.min);
        data.daily[i].temp.max = celsius_to_fahrenheit(data.daily[i].temp.max);
    }
#elif defined(UNITS_TEMP_KELVIN)
    data.current.temp = celsius_to_kelvin(data.current.temp);
    data.current.feels_like = celsius_to_kelvin(data.current.feels_like);
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i)
    {
        data.hourly[i].temp = celsius_to_kelvin(data.hourly[i].temp);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i)
    {
        data.daily[i].temp.min = celsius_to_kelvin(data.daily[i].temp.min);
        data.daily[i].temp.max = celsius_to_kelvin(data.daily[i].temp.max);
    }
#endif

#if defined(UNITS_SPEED_FEETPERSECOND)
    data.current.wind_speed = meterspersecond_to_feetpersecond(data.current.wind_speed);
    data.current.wind_gust = meterspersecond_to_feetpersecond(data.current.wind_gust);
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i)
    {
        data.hourly[i].wind_speed = meterspersecond_to_feetpersecond(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = meterspersecond_to_feetpersecond(data.hourly[i].wind_gust);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i)
    {
        data.daily[i].wind_speed = meterspersecond_to_feetpersecond(data.daily[i].wind_speed);
        data.daily[i].wind_gust = meterspersecond_to_feetpersecond(data.daily[i].wind_gust);
    }
#elif defined(UNITS_SPEED_KILOMETERSPERHOUR)
    data.current.wind_speed = meterspersecond_to_kilometersperhour(data.current.wind_speed);
    data.current.wind_gust = meterspersecond_to_kilometersperhour(data.current.wind_gust);
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i)
    {
        data.hourly[i].wind_speed = meterspersecond_to_kilometersperhour(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = meterspersecond_to_kilometersperhour(data.hourly[i].wind_gust);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i)
    {
        data.daily[i].wind_speed = meterspersecond_to_kilometersperhour(data.daily[i].wind_speed);
        data.daily[i].wind_gust = meterspersecond_to_kilometersperhour(data.daily[i].wind_gust);
    }
#elif defined(UNITS_SPEED_MILESPERHOUR)
    data.current.wind_speed = meterspersecond_to_milesperhour(data.current.wind_speed);
    data.current.wind_gust = meterspersecond_to_milesperhour(data.current.wind_gust);
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i)
    {
        data.hourly[i].wind_speed = meterspersecond_to_milesperhour(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = meterspersecond_to_milesperhour(data.hourly[i].wind_gust);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i)
    {
        data.daily[i].wind_speed = meterspersecond_to_milesperhour(data.daily[i].wind_speed);
        data.daily[i].wind_gust = meterspersecond_to_milesperhour(data.daily[i].wind_gust);
    }
#elif defined(UNITS_SPEED_KNOTS)
    data.current.wind_speed = meterspersecond_to_knots(data.current.wind_speed);
    data.current.wind_gust = meterspersecond_to_knots(data.current.wind_gust);
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i)
    {
        data.hourly[i].wind_speed = meterspersecond_to_knots(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = meterspersecond_to_knots(data.hourly[i].wind_gust);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i)
    {
        data.daily[i].wind_speed = meterspersecond_to_knots(data.daily[i].wind_speed);
        data.daily[i].wind_gust = meterspersecond_to_knots(data.daily[i].wind_gust);
    }
#elif defined(UNITS_SPEED_BEAUFORT)
    data.current.wind_speed = meterspersecond_to_beaufort(data.current.wind_speed);
    data.current.wind_gust = meterspersecond_to_beaufort(data.current.wind_gust);
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i)
    {
        data.hourly[i].wind_speed = meterspersecond_to_beaufort(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = meterspersecond_to_beaufort(data.hourly[i].wind_gust);
    }
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i)
    {
        data.daily[i].wind_speed = meterspersecond_to_beaufort(data.daily[i].wind_speed);
        data.daily[i].wind_gust = meterspersecond_to_beaufort(data.daily[i].wind_gust);
    }
#endif

#if defined(UNITS_PRES_PASCALS)
    data.current.pressure = hectopascals_to_pascals(data.current.pressure);
#elif defined(UNITS_PRES_MILLIMETERSOFMERCURY)
    data.current.pressure = hectopascals_to_millimetersofmercury(data.current.pressure);
#elif defined(UNITS_PRES_INCHESOFMERCURY)
    data.current.pressure = hectopascals_to_inchesofmercury(data.current.pressure);
#elif defined(UNITS_PRES_MILLIBARS)
    data.current.pressure = hectopascals_to_millibars(data.current.pressure);
#elif defined(UNITS_PRES_ATMOSPHERES)
    data.current.pressure = hectopascals_to_atmospheres(data.current.pressure);
#elif defined(UNITS_PRES_GRAMSPERSQUARECENTIMETER)
    data.current.pressure = hectopascals_to_gramspersquarecentimeter(data.current.pressure);
#elif defined(UNITS_PRES_POUNDSPERSQUAREINCH)
    data.current.pressure = hectopascals_to_poundspersquareinch(data.current.pressure);
#endif

#if defined(UNITS_DIST_MILES)
    data.current.visibility = meters_to_miles(data.current.visibility);
#elif defined(UNITS_DIST_KILOMETERS)
    data.current.visibility = meters_to_kilometers(data.current.visibility);
#endif

#if defined(UNITS_HOURLY_PRECIP_CENTIMETERS)
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i)
    {
        data.hourly[i].rain_1h = millimeters_to_centimeters(data.hourly[i].rain_1h);
        data.hourly[i].snow_1h = millimeters_to_centimeters(data.hourly[i].snow_1h);
    }
#elif defined(UNITS_HOURLY_PRECIP_INCHES)
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i)
    {
        data.hourly[i].rain_1h = millimeters_to_inches(data.hourly[i].rain_1h);
        data.hourly[i].snow_1h = millimeters_to_inches(data.hourly[i].snow_1h);
    }
#endif

#if defined(UNITS_DAILY_PRECIP_CENTIMETERS)
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i)
    {
        data.daily[i].rain = millimeters_to_centimeters(data.daily[i].rain);
        data.daily[i].snow = millimeters_to_centimeters(data.daily[i].snow);
    }
#elif defined(UNITS_DAILY_PRECIP_INCHES)
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i)
    {
        data.daily[i].rain = millimeters_to_inches(data.daily[i].rain);
        data.daily[i].snow = millimeters_to_inches(data.daily[i].snow);
    }
#endif
}

#endif
