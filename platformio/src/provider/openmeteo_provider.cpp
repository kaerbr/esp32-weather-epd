#include "config.h"
#ifdef USE_PROVIDER_OPENMETEO

#include <cstring>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "_locale.h"
#include "provider/openmeteo_provider.h"
#include "config.h"
#include "display_utils.h"
#include "model/wmo_code.h"

// WMO weather code to English description lookup
static const char *wmoDescription(int code)
{
  switch (code)
  {
  case 0:  return "Clear sky";
  case 1:  return "Mainly clear";
  case 2:  return "Partly cloudy";
  case 3:  return "Overcast";
  case 45: return "Fog";
  case 48: return "Depositing rime fog";
  case 51: return "Light drizzle";
  case 53: return "Moderate drizzle";
  case 55: return "Dense drizzle";
  case 56: return "Light freezing drizzle";
  case 57: return "Dense freezing drizzle";
  case 61: return "Slight rain";
  case 63: return "Moderate rain";
  case 65: return "Heavy rain";
  case 66: return "Light freezing rain";
  case 67: return "Heavy freezing rain";
  case 71: return "Slight snowfall";
  case 73: return "Moderate snowfall";
  case 75: return "Heavy snowfall";
  case 77: return "Snow grains";
  case 80: return "Slight rain showers";
  case 81: return "Moderate rain showers";
  case 82: return "Violent rain showers";
  case 85: return "Slight snow showers";
  case 86: return "Heavy snow showers";
  case 95: return "Thunderstorm";
  case 96: return "Thunderstorm with slight hail";
  case 99: return "Thunderstorm with heavy hail";
  default: return "Unknown";
  }
}

static void populateCondition(weather_condition_t &cond, int weatherCode)
{
  if (weatherCode <= 99)
  {
    cond.wmo_code = static_cast<WmoCode>(weatherCode);
  }
  else
  {
    cond.wmo_code = WmoCode::Unknown;
  }
  const char *desc = wmoDescription(weatherCode);
  strncpy(cond.description, desc, sizeof(cond.description) - 1);
  cond.description[sizeof(cond.description) - 1] = '\0';
}

static int deserializeForecast(const String &json, weather_data_t &data)
{
  // Filter document to minimize memory usage
  JsonDocument filter;
  filter["latitude"]           = true;
  filter["longitude"]          = true;
  filter["timezone"]           = true;
  filter["utc_offset_seconds"] = true;

  // Current
  filter["current"]["time"]                  = true;
  filter["current"]["temperature_2m"]        = true;
  filter["current"]["relative_humidity_2m"]  = true;
  filter["current"]["apparent_temperature"]  = true;
  filter["current"]["dew_point_2m"]          = true;
  filter["current"]["precipitation"]         = true;
  filter["current"]["rain"]                  = true;
  filter["current"]["showers"]               = true;
  filter["current"]["snowfall"]              = true;
  filter["current"]["weather_code"]          = true;
  filter["current"]["cloud_cover"]           = true;
  filter["current"]["pressure_msl"]          = true;
  filter["current"]["surface_pressure"]      = true;
  filter["current"]["wind_speed_10m"]        = true;
  filter["current"]["wind_direction_10m"]    = true;
  filter["current"]["wind_gusts_10m"]        = true;
  filter["current"]["visibility"]            = true;
  filter["current"]["uv_index"]              = true;

  // Hourly
  filter["hourly"]["time"]                        = true;
  filter["hourly"]["temperature_2m"]              = true;
  filter["hourly"]["relative_humidity_2m"]         = true;
  filter["hourly"]["apparent_temperature"]         = true;
  filter["hourly"]["dew_point_2m"]                = true;
  filter["hourly"]["precipitation_probability"]   = true;
  filter["hourly"]["rain"]                        = true;
  filter["hourly"]["snowfall"]                    = true;
  filter["hourly"]["weather_code"]                = true;
  filter["hourly"]["cloud_cover"]                 = true;
  filter["hourly"]["pressure_msl"]                = true;
  filter["hourly"]["visibility"]                  = true;
  filter["hourly"]["wind_speed_10m"]              = true;
  filter["hourly"]["wind_direction_10m"]          = true;
  filter["hourly"]["wind_gusts_10m"]              = true;
  filter["hourly"]["uv_index"]                    = true;

  // Daily
  filter["daily"]["time"]                            = true;
  filter["daily"]["weather_code"]                    = true;
  filter["daily"]["temperature_2m_max"]              = true;
  filter["daily"]["temperature_2m_min"]              = true;
  filter["daily"]["apparent_temperature_max"]        = true;
  filter["daily"]["apparent_temperature_min"]        = true;
  filter["daily"]["sunrise"]                         = true;
  filter["daily"]["sunset"]                          = true;
  filter["daily"]["uv_index_max"]                    = true;
  filter["daily"]["precipitation_sum"]               = true;
  filter["daily"]["rain_sum"]                        = true;
  filter["daily"]["snowfall_sum"]                    = true;
  filter["daily"]["precipitation_hours"]             = true;
  filter["daily"]["precipitation_probability_max"]   = true;
  filter["daily"]["wind_speed_10m_max"]              = true;
  filter["daily"]["wind_gusts_10m_max"]              = true;
  filter["daily"]["wind_direction_10m_dominant"]     = true;
  filter["daily"]["mean_relative_humidity_2m"]       = true;
  filter["daily"]["mean_dewpoint_2m"]                = true;
  filter["daily"]["mean_sea_level_pressure"]         = true;
  filter["daily"]["mean_visibility"]                 = true;

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

  // Metadata
  data.lat             = doc["latitude"]          .as<float>();
  data.lon             = doc["longitude"]         .as<float>();
  const char *tz = doc["timezone"].as<const char *>();
  if (tz)
  {
    strncpy(data.timezone, tz, sizeof(data.timezone) - 1);
    data.timezone[sizeof(data.timezone) - 1] = '\0';
  }
  data.timezone_offset = doc["utc_offset_seconds"].as<int>();

  // Current conditions
  JsonObject current = doc["current"];
  data.current.dt         = current["time"]                 .as<int64_t>();
  data.current.temp       = current["temperature_2m"]       .as<float>() + 273.15f;
  data.current.feels_like = current["apparent_temperature"] .as<float>() + 273.15f;
  data.current.pressure   = current["pressure_msl"]         .as<int>();
  data.current.humidity   = current["relative_humidity_2m"] .as<int>();
  data.current.dew_point  = current["dew_point_2m"]         .as<float>() + 273.15f;
  data.current.uvi        = current["uv_index"]             .as<float>();
  data.current.visibility = current["visibility"]           .as<int>();
  data.current.wind_speed = current["wind_speed_10m"]       .as<float>();
  data.current.wind_gust  = current["wind_gusts_10m"]       .as<float>();
  data.current.wind_deg   = current["wind_direction_10m"]   .as<int>();
  data.current.rain_1h    = current["rain"]                 .as<float>();
  data.current.snow_1h    = current["snowfall"]             .as<float>() * 10.0f; // cm → mm
  data.current.condition.clouds = current["cloud_cover"]    .as<int>();
  populateCondition(data.current.condition, current["weather_code"].as<int>());

  // Copy sunrise/sunset from daily[0] into current
  data.current.sunrise = doc["daily"]["sunrise"][0].as<int64_t>();
  data.current.sunset  = doc["daily"]["sunset"][0] .as<int64_t>();

  // Hourly forecast
  JsonArray hourlyTime    = doc["hourly"]["time"];
  JsonArray hourlyTemp    = doc["hourly"]["temperature_2m"];
  JsonArray hourlyFeels   = doc["hourly"]["apparent_temperature"];
  JsonArray hourlyPres    = doc["hourly"]["pressure_msl"];
  JsonArray hourlyHum     = doc["hourly"]["relative_humidity_2m"];
  JsonArray hourlyDew     = doc["hourly"]["dew_point_2m"];
  JsonArray hourlyUvi     = doc["hourly"]["uv_index"];
  JsonArray hourlyVis     = doc["hourly"]["visibility"];
  JsonArray hourlyWspd    = doc["hourly"]["wind_speed_10m"];
  JsonArray hourlyWgust   = doc["hourly"]["wind_gusts_10m"];
  JsonArray hourlyWdir    = doc["hourly"]["wind_direction_10m"];
  JsonArray hourlyPop     = doc["hourly"]["precipitation_probability"];
  JsonArray hourlyRain    = doc["hourly"]["rain"];
  JsonArray hourlySnow    = doc["hourly"]["snowfall"];
  JsonArray hourlyCode    = doc["hourly"]["weather_code"];
  JsonArray hourlyClouds  = doc["hourly"]["cloud_cover"];

  int numHourly = hourlyTime.size();
  if (numHourly > MAX_HOURLY) numHourly = MAX_HOURLY;
  for (int i = 0; i < numHourly; ++i)
  {
    data.hourly[i].dt         = hourlyTime[i]  .as<int64_t>();
    data.hourly[i].temp       = hourlyTemp[i]  .as<float>() + 273.15f;
    data.hourly[i].feels_like = hourlyFeels[i] .as<float>() + 273.15f;
    data.hourly[i].pressure   = hourlyPres[i]  .as<int>();
    data.hourly[i].humidity   = hourlyHum[i]   .as<int>();
    data.hourly[i].dew_point  = hourlyDew[i]   .as<float>() + 273.15f;
    data.hourly[i].uvi        = hourlyUvi[i]   .as<float>();
    data.hourly[i].visibility = hourlyVis[i]   .as<int>();
    data.hourly[i].wind_speed = hourlyWspd[i]  .as<float>();
    data.hourly[i].wind_gust  = hourlyWgust[i] .as<float>();
    data.hourly[i].wind_deg   = hourlyWdir[i]  .as<int>();
    data.hourly[i].pop        = hourlyPop[i]   .as<float>() / 100.0f;
    data.hourly[i].rain_1h    = hourlyRain[i]  .as<float>();
    data.hourly[i].snow_1h    = hourlySnow[i]  .as<float>() * 10.0f; // cm → mm
    data.hourly[i].condition.clouds = hourlyClouds[i].as<int>();
    populateCondition(data.hourly[i].condition, hourlyCode[i].as<int>());
  }

  // Daily forecast
  JsonArray dailyTime     = doc["daily"]["time"];
  JsonArray dailySunrise  = doc["daily"]["sunrise"];
  JsonArray dailySunset   = doc["daily"]["sunset"];
  JsonArray dailyCode     = doc["daily"]["weather_code"];
  JsonArray dailyTempMax  = doc["daily"]["temperature_2m_max"];
  JsonArray dailyTempMin  = doc["daily"]["temperature_2m_min"];
  JsonArray dailyAppMax   = doc["daily"]["apparent_temperature_max"];
  JsonArray dailyAppMin   = doc["daily"]["apparent_temperature_min"];
  JsonArray dailyUvi      = doc["daily"]["uv_index_max"];
  JsonArray dailyPrecip   = doc["daily"]["precipitation_sum"];
  JsonArray dailyRain     = doc["daily"]["rain_sum"];
  JsonArray dailySnow     = doc["daily"]["snowfall_sum"];
  JsonArray dailyPop      = doc["daily"]["precipitation_probability_max"];
  JsonArray dailyWspd     = doc["daily"]["wind_speed_10m_max"];
  JsonArray dailyWgust    = doc["daily"]["wind_gusts_10m_max"];
  JsonArray dailyWdir     = doc["daily"]["wind_direction_10m_dominant"];
  JsonArray dailyHum      = doc["daily"]["mean_relative_humidity_2m"];
  JsonArray dailyDew      = doc["daily"]["mean_dewpoint_2m"];
  JsonArray dailyPres     = doc["daily"]["mean_sea_level_pressure"];
  JsonArray dailyVis      = doc["daily"]["mean_visibility"];

  int numDaily = dailyTime.size();
  if (numDaily > MAX_DAILY) numDaily = MAX_DAILY;
  for (int i = 0; i < numDaily; ++i)
  {
    data.daily[i].dt         = dailyTime[i]   .as<int64_t>();
    data.daily[i].sunrise    = dailySunrise[i].as<int64_t>();
    data.daily[i].sunset     = dailySunset[i] .as<int64_t>();
    // moonrise, moonset, moon_phase: NOT AVAILABLE — remain at sentinel values
    data.daily[i].temp.min   = dailyTempMin[i].as<float>() + 273.15f;
    data.daily[i].temp.max   = dailyTempMax[i].as<float>() + 273.15f;
    // temp.morn/day/eve/night: NOT AVAILABLE — remain at NAN
    // feels_like.morn/day/eve/night: NOT AVAILABLE — remain at NAN
    data.daily[i].pressure   = dailyPres[i]   .as<int>();
    data.daily[i].humidity   = dailyHum[i]    .as<int>();
    data.daily[i].dew_point  = dailyDew[i]    .as<float>() + 273.15f;
    data.daily[i].uvi        = dailyUvi[i]    .as<float>();
    data.daily[i].visibility = dailyVis[i]    .as<int>();
    data.daily[i].wind_speed = dailyWspd[i]   .as<float>();
    data.daily[i].wind_gust  = dailyWgust[i]  .as<float>();
    data.daily[i].wind_deg   = dailyWdir[i]   .as<int>();
    data.daily[i].pop        = dailyPop[i]    .as<float>() / 100.0f;
    data.daily[i].rain       = dailyRain[i]   .as<float>();
    data.daily[i].snow       = dailySnow[i]   .as<float>() * 10.0f; // cm → mm
    // condition.clouds: NOT AVAILABLE at daily level — remains at INT32_MIN
    populateCondition(data.daily[i].condition, dailyCode[i].as<int>());
  }

  return 0;
}

static int deserializeAirQuality(const String &json, weather_data_t &data)
{
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

  JsonArray aqTime = doc["hourly"]["time"];
  JsonArray aqCo   = doc["hourly"]["carbon_monoxide"];
  JsonArray aqNo   = doc["hourly"]["nitrogen_monoxide"];
  JsonArray aqNo2  = doc["hourly"]["nitrogen_dioxide"];
  JsonArray aqO3   = doc["hourly"]["ozone"];
  JsonArray aqSo2  = doc["hourly"]["sulphur_dioxide"];
  JsonArray aqPm25 = doc["hourly"]["pm2_5"];
  JsonArray aqPm10 = doc["hourly"]["pm10"];
  JsonArray aqNh3  = doc["hourly"]["ammonia"];

  int numHours = aqTime.size();
  if (numHours > MAX_AQ_HOURS) numHours = MAX_AQ_HOURS;
  for (int i = 0; i < numHours; ++i)
  {
    data.air_quality.dt[i]    = aqTime[i].as<int64_t>();
    data.air_quality.co[i]    = aqCo[i]  .as<float>();
    data.air_quality.no[i]    = aqNo[i]  .as<float>();
    data.air_quality.no2[i]   = aqNo2[i] .as<float>();
    data.air_quality.o3[i]    = aqO3[i]  .as<float>();
    data.air_quality.so2[i]   = aqSo2[i] .as<float>();
    data.air_quality.pm2_5[i] = aqPm25[i].as<float>();
    data.air_quality.pm10[i]  = aqPm10[i].as<float>();
    data.air_quality.nh3[i]   = aqNh3[i] .as<float>();
    ++data.num_aq_hours;
  }

  return 0;
}

OpenMeteoProvider::OpenMeteoProvider(WiFiClient &client) : WeatherProvider(client)
{
  providerName = "Open-Meteo";
}

int OpenMeteoProvider::fetchData(weather_data_t &data)
{
  data.provider_name = providerName.c_str();

  // --- Forecast API ---
  int attempts = 0;
  bool rxSuccess = false;

  // "api.open-meteo.com/v1/forecast?latitude=52.52&longitude=13.41&current=temperature_2m,relative_humidity_2m,apparent_temperature,dew_point_2m,precipitation,rain,showers,snowfall,weather_code,cloud_cover,pressure_msl,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m,visibility,uv_index&hourly=temperature_2m,relative_humidity_2m,apparent_temperature,dew_point_2m,precipitation_probability,rain,snowfall,weather_code,cloud_cover,pressure_msl,visibility,wind_speed_10m,wind_direction_10m,wind_gusts_10m,uv_index&daily=weather_code,temperature_2m_max,temperature_2m_min,apparent_temperature_max,apparent_temperature_min,sunrise,sunset,uv_index_max,precipitation_sum,rain_sum,snowfall_sum,precipitation_hours,precipitation_probability_max,wind_speed_10m_max,wind_gusts_10m_max,wind_direction_10m_dominant,mean_relative_humidity_2m,mean_dewpoint_2m,mean_sea_level_pressure,mean_visibility&timezone=auto&wind_speed_unit=ms&timeformat=unixtime&forecast_days=8&forecast_hours=48";

  String endpoint = "api.open-meteo.com";
  String uri = "/v1/forecast?latitude=" + LAT + "&longitude=" + LON
    + "&current=temperature_2m,relative_humidity_2m,apparent_temperature,"
      "dew_point_2m,precipitation,rain,showers,snowfall,weather_code,"
      "cloud_cover,pressure_msl,surface_pressure,wind_speed_10m,"
      "wind_direction_10m,wind_gusts_10m,visibility,uv_index"
    + "&hourly=temperature_2m,relative_humidity_2m,apparent_temperature,"
      "dew_point_2m,precipitation_probability,rain,snowfall,weather_code,"
      "cloud_cover,pressure_msl,visibility,wind_speed_10m,"
      "wind_direction_10m,wind_gusts_10m,uv_index"
    + "&daily=weather_code,temperature_2m_max,temperature_2m_min,"
      "apparent_temperature_max,apparent_temperature_min,sunrise,sunset,"
      "uv_index_max,precipitation_sum,rain_sum,snowfall_sum,"
      "precipitation_hours,precipitation_probability_max,"
      "wind_speed_10m_max,wind_gusts_10m_max,wind_direction_10m_dominant"
    + "&timezone=auto&wind_speed_unit=ms&timeformat=unixtime"
      "&forecast_days=8&forecast_hours=48";

  String sanitizedUri = endpoint + uri;

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
    http.begin(wifi_client, endpoint, PORT, uri);
    httpResponse = http.GET();
    if (httpResponse == HTTP_CODE_OK)
    {
      String payload = http.getString();
      int parseResult = deserializeForecast(payload, data);
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

  // --- Air Quality API ---
  int weatherHttpResponse = httpResponse;
  attempts = 0;
  rxSuccess = false;

  String aqEndpoint = "air-quality-api.open-meteo.com";
  String aqUri = "/v1/air-quality?latitude=" + LAT + "&longitude=" + LON
    + "&hourly=pm10,pm2_5,carbon_monoxide,nitrogen_dioxide,"
      "sulphur_dioxide,ozone,ammonia,nitrogen_monoxide"
    + "&timezone=auto&timeformat=unixtime"
      "&past_hours=24&forecast_hours=0";

  Serial.print(TXT_ATTEMPTING_HTTP_REQ);
  Serial.println(": " + aqEndpoint + aqUri);
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
    http.begin(wifi_client, aqEndpoint, PORT, aqUri);
    httpResponse = http.GET();
    if (httpResponse == HTTP_CODE_OK)
    {
      String payload = http.getString();
      int parseResult = deserializeAirQuality(payload, data);
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

  // AQ failure is non-fatal — return weather success if weather succeeded
  if (!rxSuccess)
  {
    return weatherHttpResponse;
  }

  return httpResponse;
}

#endif
