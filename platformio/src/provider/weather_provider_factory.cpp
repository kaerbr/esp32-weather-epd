#include "provider/weather_provider_factory.h"
#include "config.h"

#include "provider/owm_provider.h"
#include "provider/openmeteo_provider.h"

WeatherProvider* WeatherProviderFactory::createProvider(WiFiClient &client)
{
#if defined(WEATHER_PROVIDER_OWM)
  return new OpenWeatherMapProvider(client);
#elif defined(WEATHER_PROVIDER_OPENMETEO)
  return new OpenMeteoProvider(client);
#else
  #error No weather provider selected. Define a WEATHER_PROVIDER_* macro in config.h.
#endif
}
