#pragma once

#include "model/weather_data.h"
#include "config.h"
#include <WiFiClient.h>

/**
 * @brief An abstract base class (interface) for weather data providers.
 *
 * Any class that provides weather data from a specific API (like OpenWeatherMap, DWD, Open-Meteo, etc.)
 * must inherit from this class and implement its pure virtual functions.
 */
class WeatherProvider
{
public:
  WeatherProvider(WiFiClient &client) : wifi_client(client) {}
  virtual ~WeatherProvider() = default;

  virtual int fetchData(weather_data_t &data) = 0;

  String providerName;

#ifdef USE_HTTP
  static const uint16_t PORT = 80;
#else
  static const uint16_t PORT = 443;
#endif

protected:
  WiFiClient &wifi_client;
};
