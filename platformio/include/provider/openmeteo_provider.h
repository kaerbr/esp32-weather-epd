#include "config.h"
#ifdef USE_PROVIDER_OPENMETEO

#pragma once

#include "weather_provider.h"

class OpenMeteoProvider : public WeatherProvider
{
public:
  explicit OpenMeteoProvider(WiFiClient &client);
  int fetchData(weather_data_t &data) override;
};

#endif
