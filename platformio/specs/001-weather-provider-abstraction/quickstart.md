# Quickstart: Weather Provider Abstraction

**Date**: 2026-04-03
**Feature**: 001-weather-provider-abstraction

## What Changed

The weather data fetching layer has been decoupled from the rendering
layer. Instead of the renderer consuming OpenWeatherMap-specific types
(`owm_current_t`, `owm_daily_t`, etc.), it now consumes a universal
`weather_data_t` structure. Weather providers implement a
`WeatherProvider` interface to populate this structure.

## For Existing Users (OpenWeatherMap)

**Nothing changes for you.** OpenWeatherMap is the default provider.
Your `config.h` works as before. The only visible change is the
provider name appearing in the status bar.

## For Contributors Adding a New Provider

### Step 1: Create the provider header

Create `include/providers/<name>_provider.h`:

```cpp
#ifndef __<NAME>_PROVIDER_H__
#define __<NAME>_PROVIDER_H__

#include "weather_provider.h"

class MyProvider : public WeatherProvider {
public:
  const char* getName() const override;
  int fetchData(WiFiClientSecure &client, weather_data_t &data) override;
};

#endif
```

### Step 2: Implement the provider

Create `src/providers/<name>_provider.cpp`:

```cpp
#include "providers/<name>_provider.h"
#include "config.h"
#include "wmo_codes.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>

const char* MyProvider::getName() const {
  return "My Weather Service";
}

int MyProvider::fetchData(WiFiClientSecure &client, weather_data_t &data) {
  data.provider_name = getName();

  // 1. Build your API URL using config values
  // 2. Make HTTP request
  // 3. Deserialize JSON
  // 4. Map fields to weather_data_t (leave unsupported fields as sentinels)
  // 5. Translate your condition codes to wmo_code_t
  // 6. Return HTTP_CODE_OK on success, error code on failure

  return HTTP_CODE_OK;
}
```

### Step 3: Register in config.h

Add your provider macro to the provider selection block:

```cpp
// WEATHER PROVIDER
#define WEATHER_PROVIDER_OWM
// #define WEATHER_PROVIDER_MYSERVICE
```

Add your provider-specific config (API key, etc.) guarded by the macro:

```cpp
#ifdef WEATHER_PROVIDER_MYSERVICE
  extern const char *MY_API_KEY;
  extern const char *MY_ENDPOINT;
#endif
```

### Step 4: Add instantiation in main.cpp

Add your provider to the conditional block:

```cpp
#elif defined(WEATHER_PROVIDER_MYSERVICE)
  #include "providers/myservice_provider.h"
  static MyProvider weatherProvider;
```

### Step 5: Update config validation

Add your macro to the XOR validation block at the bottom of `config.h`.

## Key Rules for Provider Authors

1. **Call `initWeatherData(data)` is done by the caller** -- your
   `fetchData` receives a pre-initialized struct with all sentinels.
2. **Only overwrite fields you can supply.** Leave everything else at
   sentinel values. The renderer handles missing data automatically.
3. **Translate condition codes to WMO 4677.** Use `wmo_codes.h` for
   the enum. See `owm_provider.cpp` for a mapping example.
4. **Do not use Arduino String.** Use `snprintf()` into the fixed
   `char[]` buffers in the data struct.
5. **Respect timeouts.** Use `HTTP_CLIENT_TCP_TIMEOUT` from config.
6. **Clean up HTTP resources.** Call `client.stop()` and `http.end()`
   before returning.

## Verification

After adding a provider, verify:

- [ ] Compiles with `-Wall` and no warnings
- [ ] Device wakes, fetches data, renders display, enters deep sleep
- [ ] Provider name appears in the status bar
- [ ] Missing metrics show `--` on display
- [ ] Weather condition icons are correct for the provider's data
- [ ] Switching back to `WEATHER_PROVIDER_OWM` still works identically
