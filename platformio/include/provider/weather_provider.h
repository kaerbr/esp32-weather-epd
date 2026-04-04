#pragma once

#include "model/weather_data.h"
#include "config.h"
#include <WiFiClient.h>

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
