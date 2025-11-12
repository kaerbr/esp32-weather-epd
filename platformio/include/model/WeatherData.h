/* Generic weather data model for esp32-weather-epd.
 * This file defines a standardized, API-agnostic data structure for weather information.
 */

#pragma once

#include <cstdint>
#include <vector>
#include <Arduino.h>

// Constants defining the size of forecast arrays, mirroring the original API response capabilities.
#define MAX_HOURLY_FORECASTS 48
#define MAX_DAILY_FORECASTS 8
#define MAX_ALERTS 8

// Represents a generic weather condition.
struct WeatherCondition
{
  int id;             // Weather condition id
  String main;        // Group of weather parameters (e.g., Rain, Snow, Extreme)
  String description; // Weather condition within the group
  String icon;        // Weather icon id
};

// Represents the current weather conditions.
struct CurrentWeather
{
  int64_t dt;       // Current time, Unix, UTC
  int64_t sunrise;  // Sunrise time, Unix, UTC
  int64_t sunset;   // Sunset time, Unix, UTC
  float temp;       // Temperature, in units defined by config.h
  float feels_like; // "Feels like" temperature, in units defined by config.h
  int pressure;     // Atmospheric pressure on the sea level, in units defined by config.h
  int humidity;     // Humidity, %
  float uvi;        // UV index
  int visibility;   // Average visibility, in units defined by config.h
  float wind_speed; // Wind speed, in units defined by config.h
  float wind_gust;  // Wind gust, in units defined by config.h
  int wind_deg;     // Wind direction, degrees (meteorological)
  int clouds;       // Cloudiness, %
  WeatherCondition weather;
};

// Represents a single point in the hourly forecast.
struct HourlyWeather
{
  int64_t dt;       // Time of the forecasted data, Unix, UTC
  float temp;       // Temperature, in units defined by config.h
  float pop;        // Probability of precipitation (0.0 to 1.0)
  float rain_1h;    // Rain volume for the last hour, in units defined by config.h
  float snow_1h;    // Snow volume for the last hour, in units defined by config.h
  int clouds;       // Cloudiness, %
  float wind_speed; // Wind speed, in units defined by config.h
  float wind_gust;  // Wind gust, in units defined by config.h
  WeatherCondition weather;
};

// Represents the temperature forecast for a single day.
struct DailyTemp
{
  float min; // Min daily temperature, in units defined by config.h
  float max; // Max daily temperature, in units defined by config.h
};

// Represents a single day in the daily forecast.
struct DailyWeather
{
  int64_t dt;       // Time of the forecasted data, Unix, UTC
  int64_t sunrise;  // Sunrise time, Unix, UTC
  int64_t sunset;   // Sunset time, Unix, UTC
  int64_t moonrise; // Moonrise time, Unix, UTC
  int64_t moonset;  // Moonset time, Unix, UTC
  float moon_phase; // Moon phase
  DailyTemp temp;
  float pop;        // Probability of precipitation (0.0 to 1.0)
  float rain;       // Precipitation volume, in units defined by config.h
  float snow;       // Snow volume, in units defined by config.h
  int clouds;       // Cloudiness, %
  float wind_speed; // Wind speed, in units defined by config.h
  float wind_gust;  // Wind gust, in units defined by config.h
  WeatherCondition weather;
};

// Represents a national weather alert.
struct WeatherAlert
{
  String event;  // Alert event name
  String tags;   // Type of severe weather
  int64_t start; // Date and time of the start of the alert, Unix, UTC
  int64_t end;   // Date and time of the end of the alert, Unix, UTC
};

// Represents air quality data.
struct AirQuality
{
  int aqi; // Calculated Air Quality Index
           // To keep it generic, we can store the raw component values if a provider offers them.
           // This is optional and depends on whether we want to do calculations outside the provider.
           // For now, a pre-calculated AQI is sufficient for the renderer.
};

// The main container for all weather-related data, passed from a WeatherProvider to the renderer.
struct WeatherData
{
  CurrentWeather current;
  HourlyWeather hourly[MAX_HOURLY_FORECASTS];
  DailyWeather daily[MAX_DAILY_FORECASTS];
  std::vector<WeatherAlert> alerts;
  AirQuality air_quality;

  // Metadata
  float lat;
  float lon;
  String timezone;
  int timezone_offset;
};
