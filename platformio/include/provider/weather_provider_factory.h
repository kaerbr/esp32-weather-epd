#pragma once

#include "weather_provider.h"
#include <WiFiClient.h>

class WeatherProviderFactory
{
public:
  static WeatherProvider* createProvider(WiFiClient &client);
};
