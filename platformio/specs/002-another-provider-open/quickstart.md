# Quickstart: Open-Meteo Weather Provider

## Files to Create

1. **`include/provider/openmeteo_provider.h`** — Header declaring `OpenMeteoProvider` class inheriting from `WeatherProvider`
2. **`src/provider/openmeteo_provider.cpp`** — Full implementation: API calls, JSON parsing, unit conversions

## Files to Modify

3. **`include/config.h`**:
   - Add `#define USE_PROVIDER_OPENMETEO` (commented out) in the provider selection block (line ~26)
   - Add `#define WEATHER_PROVIDER_OPENMETEO` (commented out) near line 316
   - Add `#ifdef WEATHER_PROVIDER_OPENMETEO` block with extern declarations for optional API key
   - Update config validation `#if` at line 378 to include `WEATHER_PROVIDER_OPENMETEO`

4. **`src/config.cpp`**:
   - Add Open-Meteo API key variable (empty by default for free tier)

5. **`src/provider/weather_provider_factory.cpp`**:
   - Add `#include "provider/openmeteo_provider.h"`
   - Add `#elif defined(WEATHER_PROVIDER_OPENMETEO)` branch returning `new OpenMeteoProvider(client)`

6. **`include/cert.h`** (or equivalent):
   - Add ISRG Root X1 certificate for Open-Meteo HTTPS

## Implementation Order

1. Create header file (minimal, follows `owm_provider.h` pattern)
2. Create implementation file (follows `owm_provider.cpp` pattern):
   - Static WMO description lookup
   - `deserializeForecast()` — parse weather JSON, convert units
   - `deserializeAirQuality()` — parse AQ JSON
   - `fetchData()` — construct URLs, make HTTP calls, handle errors
3. Wire into config.h / config.cpp / factory
4. Add certificate

## Key Implementation Notes

- All temperatures from Open-Meteo (Celsius) must be converted to Kelvin (+273.15f) before storage
- Snowfall from Open-Meteo (cm) must be converted to mm (×10)
- PoP from Open-Meteo (0-100) must be converted to fraction (÷100.0f)
- Request `wind_speed_unit=ms` to match internal m/s convention
- Request `timeformat=unixtime` to get Unix timestamps
- `current.sunrise` and `current.sunset` come from `daily[0]` values
- The `aqi[]` array can be left at sentinel — renderer uses raw pollutant concentrations
- Use ArduinoJson filter document to minimize memory during parsing
- Guard entire .cpp with `#ifdef USE_PROVIDER_OPENMETEO` (matching OWM pattern)
- Guard header with `#ifdef USE_PROVIDER_OPENMETEO` (matching OWM pattern)
