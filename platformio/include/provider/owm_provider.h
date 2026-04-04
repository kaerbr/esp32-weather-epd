#pragma once

#include "weather_provider.h"

class OpenWeatherMapProvider : public WeatherProvider
{
public:
  explicit OpenWeatherMapProvider(WiFiClient &client);
  int fetchData(weather_data_t &data) override;
};
