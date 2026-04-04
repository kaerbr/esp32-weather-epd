/* OpenWeatherMap provider implementation for esp32-weather-epd.
 * Copyright (C) 2022-2026  Luke Marzen
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

#include <cstring>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "_locale.h"
#include "providers/owm_provider.h"
#include "config.h"
#include "display_utils.h"
#include "wmo_codes.h"


static wmo_code_t owmIdToWmo(int owmId)
{
  switch (owmId)
  {
  // Group 2xx: Thunderstorm
  case 200: case 201: case 210: case 211: case 221: case 230: case 231:
    return WMO_THUNDERSTORM_SLIGHT_OR_MODERATE;
  case 202: case 212: case 232:
    // OWM doesn't specify hail, but this is the best approximation for severe thunderstorms
    return WMO_THUNDERSTORM_HAIL_SLIGHT;

  // Group 3xx: Drizzle
  case 300: case 310:
    return WMO_DRIZZLE_LIGHT;
  case 301: case 311: case 313: case 321:
    return WMO_DRIZZLE_MODERATE;
  case 302: case 312: case 314:
    return WMO_DRIZZLE_DENSE;

  // Group 5xx: Rain
  case 500:
    return WMO_RAIN_SLIGHT;
  case 501:
    return WMO_RAIN_MODERATE;
  case 502: case 503: case 504:
    return WMO_RAIN_HEAVY;
  case 511:
    return WMO_RAIN_FREEZING_LIGHT;
  case 520:
    return WMO_SHOWERS_RAIN_SLIGHT;
  case 521: case 531:
    return WMO_SHOWERS_RAIN_MODERATE;
  case 522:
    return WMO_SHOWERS_RAIN_VIOLENT;

  // Group 6xx: Snow
  case 600: case 615: // light snow, light rain and snow
    return WMO_SNOW_SLIGHT;
  case 601: case 616: // snow, rain and snow
    return WMO_SNOW_MODERATE;
  case 602: // heavy snow
    return WMO_SNOW_HEAVY;
  case 611: case 612: case 613: // sleet variants
    return WMO_SNOW_GRAINS;
  case 620: // light shower snow
    return WMO_SHOWERS_SNOW_SLIGHT;
  case 621: case 622: // shower snow, heavy shower snow
    return WMO_SHOWERS_SNOW_HEAVY;

  // Group 7xx: Atmosphere
  case 701: // Mist
  case 741: // Fog
    return WMO_FOG;
  case 711: // Smoke
  case 721: // Haze
  case 731: // Dust
  case 751: // Sand
  case 761: // Dust
  case 762: // Ash
  case 771: // Squall
  case 781: // Tornado
    return WMO_UNKNOWN; // The simplified Open-Meteo standard doesn't have exact codes for these.

  // Group 800: Clear
  case 800:
    return WMO_CLEAR_SKY;

  // Group 80x: Clouds
  case 801: // 11-25%
    return WMO_MAINLY_CLEAR;
  case 802: // 25-50%
    return WMO_PARTLY_CLOUDY;
  case 803: // 51-84%
  case 804: // 85-100%
    return WMO_OVERCAST;

  default:
    // Fallback by group range
    if (owmId >= 200 && owmId < 300) return WMO_THUNDERSTORM_SLIGHT_OR_MODERATE;
    if (owmId >= 300 && owmId < 400) return WMO_DRIZZLE_MODERATE;
    if (owmId >= 500 && owmId < 600) return WMO_RAIN_MODERATE;
    if (owmId >= 600 && owmId < 700) return WMO_SNOW_MODERATE;
    if (owmId == 701 || owmId == 741) return WMO_FOG;
    if (owmId > 700 && owmId < 800) return WMO_UNKNOWN; 
    if (owmId > 800 && owmId < 900) return WMO_OVERCAST;

    return WMO_UNKNOWN;
  }
}

static void populateCondition(weather_condition_t &cond, JsonObject &weather)
{
  int owmId = weather["id"].as<int>();
  cond.wmo_code = owmIdToWmo(owmId);
  const char *desc = weather["description"].as<const char *>();
  if (desc)
  {
    strncpy(cond.description, desc, sizeof(cond.description) - 1);
    cond.description[sizeof(cond.description) - 1] = '\0';
  }
}

static int deserializeOneCall(WiFiClient &json, weather_data_t &data)
{
  int i;

  JsonDocument filter;
  filter["current"]  = true;
  filter["minutely"] = false;
  filter["hourly"]   = true;
  filter["daily"]    = true;
#if !DISPLAY_ALERTS
  filter["alerts"]   = false;
#else
  for (int j = 0; j < MAX_ALERTS; ++j)
  {
    filter["alerts"][j]["sender_name"] = false;
    filter["alerts"][j]["event"]       = true;
    filter["alerts"][j]["start"]       = true;
    filter["alerts"][j]["end"]         = true;
    filter["alerts"][j]["description"] = false;
    filter["alerts"][j]["tags"]        = true;
  }
#endif

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json,
                                         DeserializationOption::Filter(filter));
#if DEBUG_LEVEL >= 1
  Serial.println("[debug] doc.overflowed() : "
                 + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
  serializeJsonPretty(doc, Serial);
#endif
  if (error)
  {
    return -256 - static_cast<int>(error.code());
  }

  data.lat             = doc["lat"]            .as<float>();
  data.lon             = doc["lon"]            .as<float>();
  const char *tz = doc["timezone"].as<const char *>();
  if (tz)
  {
    strncpy(data.timezone, tz, sizeof(data.timezone) - 1);
    data.timezone[sizeof(data.timezone) - 1] = '\0';
  }
  data.timezone_offset = doc["timezone_offset"].as<int>();

  JsonObject current = doc["current"];
  data.current.dt         = current["dt"]        .as<int64_t>();
  data.current.sunrise    = current["sunrise"]   .as<int64_t>();
  data.current.sunset     = current["sunset"]    .as<int64_t>();
  data.current.temp       = current["temp"]      .as<float>();
  data.current.feels_like = current["feels_like"].as<float>();
  data.current.pressure   = current["pressure"]  .as<int>();
  data.current.humidity   = current["humidity"]  .as<int>();
  data.current.dew_point  = current["dew_point"] .as<float>();
  data.current.uvi        = current["uvi"]       .as<float>();
  data.current.visibility = current["visibility"].as<int>();
  data.current.wind_speed = current["wind_speed"].as<float>();
  data.current.wind_gust  = current["wind_gust"] .as<float>();
  data.current.wind_deg   = current["wind_deg"]  .as<int>();
  data.current.rain_1h    = current["rain"]["1h"].as<float>();
  data.current.snow_1h    = current["snow"]["1h"].as<float>();
  data.current.condition.clouds = current["clouds"].as<int>();
  JsonObject current_weather = current["weather"][0];
  populateCondition(data.current.condition, current_weather);

  i = 0;
  for (JsonObject hourly : doc["hourly"].as<JsonArray>())
  {
    data.hourly[i].dt         = hourly["dt"]        .as<int64_t>();
    data.hourly[i].temp       = hourly["temp"]      .as<float>();
    data.hourly[i].feels_like = hourly["feels_like"].as<float>();
    data.hourly[i].pressure   = hourly["pressure"]  .as<int>();
    data.hourly[i].humidity   = hourly["humidity"]  .as<int>();
    data.hourly[i].dew_point  = hourly["dew_point"] .as<float>();
    data.hourly[i].uvi        = hourly["uvi"]       .as<float>();
    data.hourly[i].visibility = hourly["visibility"].as<int>();
    data.hourly[i].wind_speed = hourly["wind_speed"].as<float>();
    data.hourly[i].wind_gust  = hourly["wind_gust"] .as<float>();
    data.hourly[i].wind_deg   = hourly["wind_deg"]  .as<int>();
    data.hourly[i].pop        = hourly["pop"]       .as<float>();
    data.hourly[i].rain_1h    = hourly["rain"]["1h"].as<float>();
    data.hourly[i].snow_1h    = hourly["snow"]["1h"].as<float>();
    data.hourly[i].condition.clouds = hourly["clouds"].as<int>();
    JsonObject hourly_weather = hourly["weather"][0];
    populateCondition(data.hourly[i].condition, hourly_weather);

    if (i == MAX_HOURLY - 1)
    {
      break;
    }
    ++i;
  }

  i = 0;
  for (JsonObject daily : doc["daily"].as<JsonArray>())
  {
    data.daily[i].dt         = daily["dt"]        .as<int64_t>();
    data.daily[i].sunrise    = daily["sunrise"]   .as<int64_t>();
    data.daily[i].sunset     = daily["sunset"]    .as<int64_t>();
    data.daily[i].moonrise   = daily["moonrise"]  .as<int64_t>();
    data.daily[i].moonset    = daily["moonset"]   .as<int64_t>();
    data.daily[i].moon_phase = daily["moon_phase"].as<float>();
    JsonObject daily_temp = daily["temp"];
    data.daily[i].temp.morn  = daily_temp["morn"] .as<float>();
    data.daily[i].temp.day   = daily_temp["day"]  .as<float>();
    data.daily[i].temp.eve   = daily_temp["eve"]  .as<float>();
    data.daily[i].temp.night = daily_temp["night"].as<float>();
    data.daily[i].temp.min   = daily_temp["min"]  .as<float>();
    data.daily[i].temp.max   = daily_temp["max"]  .as<float>();
    JsonObject daily_feels_like = daily["feels_like"];
    data.daily[i].feels_like.morn  = daily_feels_like["morn"] .as<float>();
    data.daily[i].feels_like.day   = daily_feels_like["day"]  .as<float>();
    data.daily[i].feels_like.eve   = daily_feels_like["eve"]  .as<float>();
    data.daily[i].feels_like.night = daily_feels_like["night"].as<float>();
    data.daily[i].pressure   = daily["pressure"]  .as<int>();
    data.daily[i].humidity   = daily["humidity"]  .as<int>();
    data.daily[i].dew_point  = daily["dew_point"] .as<float>();
    data.daily[i].uvi        = daily["uvi"]       .as<float>();
    data.daily[i].visibility = daily["visibility"].as<int>();
    data.daily[i].wind_speed = daily["wind_speed"].as<float>();
    data.daily[i].wind_gust  = daily["wind_gust"] .as<float>();
    data.daily[i].wind_deg   = daily["wind_deg"]  .as<int>();
    data.daily[i].pop        = daily["pop"]       .as<float>();
    data.daily[i].rain       = daily["rain"]      .as<float>();
    data.daily[i].snow       = daily["snow"]      .as<float>();
    data.daily[i].condition.clouds = daily["clouds"].as<int>();
    JsonObject daily_weather = daily["weather"][0];
    populateCondition(data.daily[i].condition, daily_weather);

    if (i == MAX_DAILY - 1)
    {
      break;
    }
    ++i;
  }

#if DISPLAY_ALERTS
  i = 0;
  for (JsonObject alerts : doc["alerts"].as<JsonArray>())
  {
    weather_alert_t &a = data.alerts[i];
    const char *event = alerts["event"].as<const char *>();
    if (event)
    {
      strncpy(a.event, event, sizeof(a.event) - 1);
      a.event[sizeof(a.event) - 1] = '\0';
    }
    a.start = alerts["start"].as<int64_t>();
    a.end   = alerts["end"]  .as<int64_t>();
    const char *tags = alerts["tags"][0].as<const char *>();
    if (tags)
    {
      strncpy(a.tags, tags, sizeof(a.tags) - 1);
      a.tags[sizeof(a.tags) - 1] = '\0';
    }
    ++data.num_alerts;

    if (i == MAX_ALERTS - 1)
    {
      break;
    }
    ++i;
  }
#endif

  return 0;
}

static int deserializeAirQuality(WiFiClient &json, weather_data_t &data)
{
  int i = 0;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
#if DEBUG_LEVEL >= 1
  Serial.println("[debug] doc.overflowed() : "
                 + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
  serializeJsonPretty(doc, Serial);
#endif
  if (error)
  {
    return -256 - static_cast<int>(error.code());
  }

  for (JsonObject list : doc["list"].as<JsonArray>())
  {
    data.air_quality.aqi[i] = list["main"]["aqi"].as<int>();

    JsonObject c = list["components"];
    data.air_quality.co[i]    = c["co"].as<float>();
    data.air_quality.no[i]    = c["no"].as<float>();
    data.air_quality.no2[i]   = c["no2"].as<float>();
    data.air_quality.o3[i]    = c["o3"].as<float>();
    data.air_quality.so2[i]   = c["so2"].as<float>();
    data.air_quality.pm2_5[i] = c["pm2_5"].as<float>();
    data.air_quality.pm10[i]  = c["pm10"].as<float>();
    data.air_quality.nh3[i]   = c["nh3"].as<float>();

    data.air_quality.dt[i] = list["dt"].as<int64_t>();
    ++data.num_aq_hours;

    if (i == MAX_AQ_HOURS - 1)
    {
      break;
    }
    ++i;
  }

  return 0;
}

OpenWeatherMapProvider::OpenWeatherMapProvider(WiFiClient &client)
  : WeatherProvider(client)
{
  providerName = "OpenWeatherMap";
}

int OpenWeatherMapProvider::fetchData(weather_data_t &data)
{
  data.provider_name = providerName.c_str();

  // --- OneCall API ---
  int attempts = 0;
  bool rxSuccess = false;
  String uri = "/data/" + OWM_ONECALL_VERSION
               + "/onecall?lat=" + LAT + "&lon=" + LON + "&lang=" + OWM_LANG
               + "&units=standard&exclude=minutely";
#if !DISPLAY_ALERTS
  uri += ",alerts";
#endif

  String sanitizedUri = OWM_ENDPOINT + uri + "&appid={API key}";
  uri += "&appid=" + OWM_APIKEY;

  Serial.print(TXT_ATTEMPTING_HTTP_REQ);
  Serial.println(": " + sanitizedUri);
  int httpResponse = 0;
  while (!rxSuccess && attempts < 3)
  {
    wl_status_t connection_status = WiFi.status();
    if (connection_status != WL_CONNECTED)
    {
      return -512 - static_cast<int>(connection_status);
    }

    HTTPClient http;
    http.setConnectTimeout(HTTP_CLIENT_TCP_TIMEOUT);
    http.setTimeout(HTTP_CLIENT_TCP_TIMEOUT);
    http.begin(wifi_client, OWM_ENDPOINT, PORT, uri);
    httpResponse = http.GET();
    if (httpResponse == HTTP_CODE_OK)
    {
      int parseResult = deserializeOneCall(http.getStream(), data);
      if (parseResult < 0)
      {
        httpResponse = parseResult;
      }
      else
      {
        rxSuccess = true;
      }
    }
    wifi_client.stop();
    http.end();
    Serial.println("  " + String(httpResponse, DEC) + " "
                   + getHttpResponsePhrase(httpResponse));
    ++attempts;
  }

  if (!rxSuccess)
  {
    return httpResponse;
  }

  // --- Air Pollution API ---
  attempts = 0;
  rxSuccess = false;

  time_t now;
  int64_t end = time(&now);
  int64_t start = end - ((3600 * MAX_AQ_HOURS) - 1);
  char endStr[22];
  char startStr[22];
  sprintf(endStr, "%lld", end);
  sprintf(startStr, "%lld", start);
  uri = "/data/2.5/air_pollution/history?lat=" + LAT + "&lon=" + LON
        + "&start=" + startStr + "&end=" + endStr
        + "&appid=" + OWM_APIKEY;
  sanitizedUri = OWM_ENDPOINT +
               "/data/2.5/air_pollution/history?lat=" + LAT + "&lon=" + LON
               + "&start=" + startStr + "&end=" + endStr
               + "&appid={API key}";

  Serial.print(TXT_ATTEMPTING_HTTP_REQ);
  Serial.println(": " + sanitizedUri);
  httpResponse = 0;
  while (!rxSuccess && attempts < 3)
  {
    wl_status_t connection_status = WiFi.status();
    if (connection_status != WL_CONNECTED)
    {
      return -512 - static_cast<int>(connection_status);
    }

    HTTPClient http;
    http.setConnectTimeout(HTTP_CLIENT_TCP_TIMEOUT);
    http.setTimeout(HTTP_CLIENT_TCP_TIMEOUT);
    http.begin(wifi_client, OWM_ENDPOINT, PORT, uri);
    httpResponse = http.GET();
    if (httpResponse == HTTP_CODE_OK)
    {
      int parseResult = deserializeAirQuality(http.getStream(), data);
      if (parseResult < 0)
      {
        httpResponse = parseResult;
      }
      else
      {
        rxSuccess = true;
      }
    }
    wifi_client.stop();
    http.end();
    Serial.println("  " + String(httpResponse, DEC) + " "
                   + getHttpResponsePhrase(httpResponse));
    ++attempts;
  }

  return httpResponse;
}
