# Research: Weather Provider Abstraction

**Date**: 2026-04-03
**Feature**: 001-weather-provider-abstraction

## R1: C++ Abstract Base Class on ESP32/Arduino

**Decision**: Use a C++ abstract base class (`WeatherProvider`) with pure
virtual methods. Instantiate the concrete provider at compile time via
preprocessor conditional in `main.cpp`.

**Rationale**: The ESP32 Arduino toolchain fully supports C++ virtual
dispatch. The overhead of a vtable pointer (~4 bytes) and one indirect
call per wake cycle is negligible compared to WiFi/HTTP latency. Compile-
time selection via `#define` in `config.h` means zero runtime overhead
for provider resolution—only the selected provider's code is linked.

**Alternatives considered**:
- **Function pointers in a struct (C-style)**: Equivalent runtime cost,
  but loses type safety and is harder for contributors to extend.
- **Compile-time templates (CRTP)**: Zero-overhead dispatch but
  significantly more complex for community contributors to implement.
  The one virtual call per wake cycle does not justify the complexity.
- **Runtime selection (string-based)**: Would require all providers to
  be compiled in, wasting flash. Rejected per power/memory principles.

## R2: WMO 4677 Code Mapping Strategy

**Decision**: Use a curated subset of WMO 4677 codes (0-99) as the
universal weather condition standard. Define an enum with ~25 grouped
categories that map to the project's existing icon set. Each provider
translates its native codes into this enum.

**Rationale**: The full WMO 4677 table has 100 codes with granularity
the E-Paper display cannot express (e.g., "fog thinning, sky visible"
vs. "fog thinning, sky invisible" both map to the same fog icon). A
grouped enum reduces mapping burden for provider authors while
preserving all distinctions the icon set can render.

**Grouped WMO categories for icon mapping**:

| Category Enum | WMO Codes | Icon Mapping |
|---------------|-----------|--------------|
| WMO_CLEAR | 0-3 | wi_day_sunny / wi_night_clear |
| WMO_HAZE | 5 | wi_day_haze / wi_dust |
| WMO_DUST | 6-9, 30-35 | wi_dust / wi_sandstorm |
| WMO_MIST | 10 | wi_day_fog / wi_night_fog |
| WMO_FOG | 11-12, 40-49 | wi_fog / wi_day_fog / wi_night_fog |
| WMO_LIGHTNING | 13, 17 | wi_lightning |
| WMO_SQUALL | 18 | wi_cloudy_gusts |
| WMO_TORNADO | 19 | wi_tornado |
| WMO_DRIZZLE | 20, 50-55, 58-59 | wi_showers / wi_day_showers |
| WMO_FREEZING_DRIZZLE | 24, 56-57 | wi_rain_mix / wi_day_rain_mix |
| WMO_RAIN | 21, 25, 60-65, 80-82 | wi_rain / wi_day_rain |
| WMO_FREEZING_RAIN | 66-67 | wi_rain_mix |
| WMO_RAIN_SNOW | 23, 68-69, 83-84 | wi_rain_mix |
| WMO_SNOW | 22, 26, 70-77, 85-86 | wi_snow / wi_day_snow |
| WMO_SLEET | 79, 87-88 | wi_sleet |
| WMO_HAIL | 27, 89-90 | wi_hail |
| WMO_THUNDERSTORM | 29, 91-94, 95, 97 | wi_thunderstorm |
| WMO_THUNDERSTORM_HAIL | 96, 98-99 | wi_thunderstorm (+ hail variant) |
| WMO_CLOUDY_FEW | N/A (provider-mapped) | wi_day_sunny_overcast |
| WMO_CLOUDY_SCATTERED | N/A (provider-mapped) | wi_day_cloudy |
| WMO_CLOUDY_BROKEN | N/A (provider-mapped) | wi_day_cloudy |
| WMO_OVERCAST | N/A (provider-mapped) | wi_cloudy |
| WMO_SMOKE | 4 | wi_smoke |
| WMO_VOLCANIC_ASH | (special) | wi_volcano |
| WMO_UNKNOWN | fallback | wi_na |

**Note**: WMO 4677 does not have explicit cloud cover percentage codes
(it's an observation code, not a forecast code). Cloud categories
(few/scattered/broken/overcast) are added as project-specific extensions
to accommodate forecast data from providers like OpenWeatherMap.

**Alternatives considered**:
- **Raw WMO code integers (0-99)**: Too granular; 100-entry switch
  statement in icon selection would be fragile and hard to maintain.
- **OpenWeatherMap codes as the standard**: Defeats the purpose of
  decoupling. Other providers would need to reverse-map to OWM codes.

## R3: OpenWeatherMap Code → WMO Mapping

**Decision**: Create a static lookup function `owmIdToWmo(int owmId)`
in the OWM provider. The mapping covers all OWM condition codes
(200-804).

**Key mappings**:

| OWM Range | OWM Description | WMO Category |
|-----------|----------------|--------------|
| 200-202 | Thunderstorm + rain | WMO_THUNDERSTORM |
| 210-221 | Thunderstorm | WMO_THUNDERSTORM |
| 230-232 | Thunderstorm + drizzle | WMO_THUNDERSTORM |
| 300-321 | Drizzle | WMO_DRIZZLE |
| 500-504 | Rain | WMO_RAIN |
| 511 | Freezing rain | WMO_FREEZING_RAIN |
| 520-531 | Shower rain | WMO_RAIN |
| 600-602 | Snow | WMO_SNOW |
| 611-613 | Sleet | WMO_SLEET |
| 615-622 | Rain + snow mix | WMO_RAIN_SNOW |
| 701 | Mist | WMO_MIST |
| 711 | Smoke | WMO_SMOKE |
| 721 | Haze | WMO_HAZE |
| 731, 751 | Sand/dust | WMO_DUST |
| 741 | Fog | WMO_FOG |
| 761 | Dust | WMO_DUST |
| 762 | Volcanic ash | WMO_VOLCANIC_ASH |
| 771 | Squall | WMO_SQUALL |
| 781 | Tornado | WMO_TORNADO |
| 800 | Clear sky | WMO_CLEAR |
| 801 | Few clouds | WMO_CLOUDY_FEW |
| 802 | Scattered clouds | WMO_CLOUDY_SCATTERED |
| 803 | Broken clouds | WMO_CLOUDY_BROKEN |
| 804 | Overcast | WMO_OVERCAST |

## R4: Day/Night Determination Without OWM Icon Strings

**Decision**: Replace `isDay(weather.icon)` with a function
`isDaytime(int64_t dt, int64_t sunrise, int64_t sunset)` that compares
the observation timestamp against sunrise/sunset.

**Rationale**: The current code uses OWM's icon suffix ("01d" → day,
"01n" → night). Since the universal data struct has sunrise/sunset
timestamps, direct comparison is both provider-agnostic and more
accurate (it works for hourly forecasts too, using the daily
sunrise/sunset of the corresponding day).

**Implementation**: For current conditions, use `current.sunrise` and
`current.sunset`. For hourly forecasts, use the corresponding day's
sunrise/sunset from the daily array. This matches the existing
`isMoonInSky()` pattern already in `display_utils.cpp`.

## R5: Sentinel Value Strategy

**Decision**: Use the following sentinel values in the universal data
structure to indicate missing/unavailable metrics:

| Type | Sentinel | Rationale |
|------|----------|-----------|
| `float` | `NAN` | Standard IEEE 754, checked with `std::isnan()` |
| `int` | `INT32_MIN` | Unlikely to be a valid weather value |
| `int64_t` (timestamps) | `0` | Unix epoch 0 = 1970-01-01, never valid for weather |
| `uint8_t` (WMO code) | `UINT8_MAX` (255) | WMO codes are 0-99 |
| `char[]` (strings) | `'\0'` (empty) | Empty string = not set |

**Renderer integration**: Each widget drawing function checks its input
value against the sentinel. If sentinel, it renders `"--"` and returns
early. This is a small change per function, consistent with the existing
pattern for NAN checks on BME sensor data.

## R6: Memory Impact Assessment

**Decision**: The universal `weather_data_t` struct will be approximately
the same size as the current combined `owm_resp_onecall_t` +
`owm_resp_air_pollution_t`. Both are statically allocated.

**Current footprint estimate**:
- `owm_resp_onecall_t`: ~48 hourly * ~80B + 8 daily * ~120B + current ~120B + alerts overhead ≈ ~5.1 KB (excluding String objects on heap)
- `owm_resp_air_pollution_t`: 24 * ~68B ≈ ~1.6 KB
- Total: ~6.7 KB static + heap for Arduino Strings

**New footprint estimate**:
- `weather_data_t`: Similar field count but replaces `String` fields with
  fixed `char[64]` arrays. Strings in alerts/descriptions add ~1 KB.
- Total: ~8 KB static, 0 heap.
- Net: slight increase in static allocation, but eliminates all heap
  fragmentation from String objects. This is a net improvement per
  Constitution Principle II.

## R7: Provider Attribution in Status Bar

**Decision**: Add a `const char*` parameter to `drawStatusBar()` for the
provider name. Display it left-aligned or after the refresh time string
in the status bar area.

**Rationale**: The status bar already has `statusStr`, `refreshTimeStr`,
`rssi`, and `batVoltage`. Adding one more string parameter is minimal
change. The provider name comes from the `WeatherProvider::getName()`
virtual method, stored as `const char*` (static string literal in each
provider).
