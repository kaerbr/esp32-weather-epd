# Contract: WeatherProvider Interface

**Date**: 2026-04-03
**Feature**: 001-weather-provider-abstraction

## Abstract Base Class: WeatherProvider

This is the interface that all weather providers MUST implement.

### Method: getName()

```
const char* getName() const
```

- **Returns**: A static string literal identifying the provider
  (e.g., `"OpenWeatherMap"`, `"WeatherAPI"`, `"Open-Meteo"`).
- **Constraints**: MUST return a pointer to a string literal or
  static const char array. MUST NOT allocate memory.
- **Used by**: Status bar rendering for provider attribution.

### Method: fetchData()

```
int fetchData(WiFiClient &client, weather_data_t &data)
```

Or with secure client:

```
int fetchData(WiFiClientSecure &client, weather_data_t &data)
```

- **Parameters**:
  - `client`: An already-initialized WiFi client (plain or secure,
    determined by config.h `USE_HTTP` / `USE_HTTPS_*` macros).
  - `data`: Reference to a pre-initialized `weather_data_t` struct
    (all fields set to sentinels by `initWeatherData()`).
- **Returns**: HTTP status code on success (`HTTP_CODE_OK` = 200),
  or a negative error code on failure. Error code conventions:
  - `-512 - wifi_status`: WiFi connection lost during fetch
  - `-256 - json_error`: JSON deserialization error
  - Standard HTTP error codes (4xx, 5xx) from the API
- **Behavior**:
  - MUST populate `data.provider_name` with `getName()` result.
  - MUST populate all fields the provider can supply with real values.
  - MUST leave fields the provider cannot supply at their sentinel values.
  - MUST translate native weather condition codes to `wmo_code_t` via
    the WMO 4677 standard before writing to `data.*.condition.wmo_code`.
  - MUST handle its own API endpoint construction, including any
    provider-specific URL paths, query parameters, and API keys.
  - MUST use `HTTP_CLIENT_TCP_TIMEOUT` from config for connection
    and read timeouts.
  - MUST NOT perform more than one retry attempt (max 3 total attempts,
    matching current OWM behavior).
  - MUST NOT enable or disable WiFi (caller responsibility).
  - MUST call `client.stop()` and `http.end()` before returning.
  - Air quality data is the provider's responsibility. If the provider
    offers air quality data, it MUST be fetched as part of this method
    and populated into `data.air_quality`. If not available, leave at
    sentinel values.

### Template for adding a new provider

A contributor adding a new provider MUST:

1. Create `include/providers/<name>_provider.h` declaring a class that
   inherits from `WeatherProvider`.
2. Create `src/providers/<name>_provider.cpp` implementing `getName()`
   and `fetchData()`.
3. Add a `#define WEATHER_PROVIDER_<NAME>` option in the provider
   selection block of `config.h`.
4. Add the conditional include and instantiation in `main.cpp`'s
   provider selection block.
5. Add any provider-specific config variables (API key, endpoint) to
   `config.h` and `config.cpp`, guarded by the provider macro.

No changes to `renderer.cpp`, `display_utils.cpp`, or `weather_data.h`
should be necessary.

## Compile-Time Provider Selection

In `config.h`:

```
// WEATHER PROVIDER
// Uncomment exactly one provider.
#define WEATHER_PROVIDER_OWM
// #define WEATHER_PROVIDER_EXAMPLE
```

In `main.cpp`, the provider is instantiated conditionally:

```
#if defined(WEATHER_PROVIDER_OWM)
  #include "providers/owm_provider.h"
  static OpenWeatherMapProvider weatherProvider;
#elif defined(WEATHER_PROVIDER_EXAMPLE)
  #include "providers/example_provider.h"
  static ExampleProvider weatherProvider;
#else
  #error "No weather provider selected. Define exactly one WEATHER_PROVIDER_* in config.h."
#endif
```

## Validation

The config validation block at the bottom of `config.h` MUST include:

```
#if !(  defined(WEATHER_PROVIDER_OWM) \
      ^ defined(WEATHER_PROVIDER_EXAMPLE) )
  #error Invalid configuration. Exactly one weather provider must be selected.
#endif
```

This block is extended as new providers are added.
