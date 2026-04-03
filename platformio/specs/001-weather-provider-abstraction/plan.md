# Implementation Plan: Weather Provider Abstraction

**Branch**: `001-weather-provider-abstraction` | **Date**: 2026-04-03 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/001-weather-provider-abstraction/spec.md`

## Summary

Decouple the esp32-weather-epd application from OpenWeatherMap by introducing
a provider-agnostic weather data architecture. An abstract base class
(`WeatherProvider`) defines the fetcher interface; a universal data structure
(`weather_data_t`) replaces all `owm_*` types consumed by the renderer.
OpenWeatherMap logic is extracted into `OpenWeatherMapProvider`. Weather
condition codes are normalized to WMO 4677 standard. The renderer and
display utilities are updated to operate exclusively on the universal types,
enabling future providers to be added without modifying rendering code.

## Technical Context

**Language/Version**: C++17 (`-std=gnu++17`) with Arduino framework
**Primary Dependencies**: GxEPD2 1.6.8, ArduinoJson 7.4.3, Adafruit BME280/BME680, WiFiClientSecure
**Storage**: NVS (Preferences) for battery state only
**Testing**: Manual (compile, flash, observe). No automated test framework.
**Target Platform**: ESP32 (Espressif32 6.13.0), DFRobot FireBeetle2 ESP32-E
**Project Type**: Embedded firmware (PlatformIO)
**Performance Goals**: Wake cycle active time <=5% increase vs. pre-refactor
**Constraints**: ~320 KB SRAM, battery-powered (6-12 month target), no Arduino String in data structures
**Scale/Scope**: Single device, 2 API calls per wake cycle (OneCall + Air Pollution for OWM)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Extreme Power Efficiency | PASS | No new network calls introduced. Provider abstraction is compile-time; no runtime dispatch overhead beyond a single virtual call. WiFi usage unchanged. |
| II. Memory Management | PASS | Universal data struct uses fixed-size arrays and `char[]` buffers, no Arduino String. Static allocation for the weather data object (same as current `owm_onecall` / `owm_air_pollution`). |
| III. Configuration Separation | PASS | Provider selection via `#define WEATHER_PROVIDER` in `config.h`. Provider-specific API keys and endpoints remain in `config.h`/`config.cpp`. |
| IV. Resilient Error Handling | PASS | Each provider's `fetchData()` returns a status code. Failure path unchanged: log, draw error, deep sleep. No retry loops added. |
| V. Modularity | PASS | This feature *improves* modularity: renderer no longer depends on OWM-specific types. Network fetching is encapsulated per-provider. |

## Project Structure

### Documentation (this feature)

```text
specs/001-weather-provider-abstraction/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output
│   └── weather_provider_interface.md
└── tasks.md             # Phase 2 output (created by /speckit.tasks)
```

### Source Code (repository root)

```text
include/
├── config.h                    # MODIFIED - add WEATHER_PROVIDER macro
├── weather_data.h              # NEW - universal weather data structs
├── weather_provider.h          # NEW - abstract base class
├── wmo_codes.h                 # NEW - WMO 4677 enum + helpers
├── providers/
│   └── owm_provider.h          # NEW - OpenWeatherMap provider declaration
├── api_response.h              # DEPRECATED (kept temporarily, replaced by weather_data.h)
├── client_utils.h              # MODIFIED - remove OWM-specific fetch functions
├── display_utils.h             # MODIFIED - change signatures from owm_* to weather_data_t
└── renderer.h                  # MODIFIED - change signatures from owm_* to weather_data_t

src/
├── config.cpp                  # MODIFIED - add provider-specific config defaults
├── weather_data.cpp            # NEW - sentinel initialization helpers
├── wmo_codes.cpp               # NEW - WMO code utility functions
├── providers/
│   └── owm_provider.cpp        # NEW - extracted OWM fetch + deserialize + code mapping
├── client_utils.cpp            # MODIFIED - retain WiFi/NTP utils, remove OWM fetch functions
├── display_utils.cpp           # MODIFIED - update icon selection to use WMO codes
├── renderer.cpp                # MODIFIED - update signatures, add provider attribution, add sentinel checks
└── main.cpp                    # MODIFIED - instantiate provider, call generic interface
```

**Structure Decision**: Flat `include/providers/` and `src/providers/`
directories for provider implementations. Each provider is a single
header + source pair. No deeper nesting needed at this scale.

## Complexity Tracking

> No constitution violations to justify.

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| N/A | N/A | N/A |
