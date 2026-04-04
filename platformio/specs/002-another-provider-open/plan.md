# Implementation Plan: Open-Meteo Weather Provider

**Branch**: `002-open-meteo-provider` | **Date**: 2026-04-05 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/002-another-provider-open/spec.md`

## Summary

Add Open-Meteo as a second weather provider, implementing the existing `WeatherProvider` abstract interface. The provider fetches current conditions, 48-hour hourly forecasts, 8-day daily forecasts, and air quality data from two Open-Meteo API endpoints. No API key is required for non-commercial use. All data is mapped to the existing `weather_data_t` structure with unit conversions (Celsius→Kelvin, cm→mm snowfall, %→fraction PoP). WMO weather codes from Open-Meteo map directly to the app's existing `wmo_code_t` enum.

## Technical Context

**Language/Version**: C++17 (`-std=gnu++17`) with Arduino framework
**Primary Dependencies**: ArduinoJson 7.4.3, HTTPClient, WiFiClientSecure, GxEPD2 1.6.8
**Storage**: N/A (no persistent storage; all data is in-memory per wake cycle)
**Testing**: Manual (compile, flash, observe on hardware) — per constitution
**Target Platform**: ESP32 (Espressif32 6.13.0, 320KB SRAM, 80MHz)
**Project Type**: Embedded firmware (battery-powered e-paper weather display)
**Performance Goals**: Complete all API calls within wake window (~10s network budget)
**Constraints**: <320KB SRAM, battery-powered (6-12mo target), compile-time provider selection
**Scale/Scope**: Single device, 2 API calls per wake cycle (forecast + air quality)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Extreme Power Efficiency | PASS | Same wake-fetch-sleep pattern as OWM. 2 API calls (identical count). No polling or delay loops. |
| II. Memory Management | PASS | ArduinoJson filter documents minimize parse memory. Static buffers for URL construction. Follow existing OWM patterns. |
| III. Configuration Separation | PASS | Provider macro in config.h. Optional API key in config.h/config.cpp. Uses existing LAT/LON/TIMEZONE globals. |
| IV. Resilient Error Handling | PASS* | Retry logic (3 attempts) matches existing OWM pattern. See Complexity Tracking. |
| V. Modularity | PASS | New files in provider/ only. No changes to renderer, display_utils, or weather_data.h. |

*\*See Complexity Tracking for retry loop justification.*

**Post-Phase 1 Re-check**: PASS — Design adds no new dependencies, no schema changes, no new memory patterns. Certificate addition is additive only.

## Project Structure

### Documentation (this feature)

```text
specs/002-another-provider-open/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── openmeteo_api_contract.md
└── tasks.md              # Phase 2 output (/speckit.tasks)
```

### Source Code (repository root)

```text
include/
├── config.h                          # MODIFIED: add provider macro + config externs
├── cert.h                            # MODIFIED: add ISRG Root X1 certificate
└── provider/
    ├── weather_provider.h            # UNCHANGED
    ├── weather_provider_factory.h    # UNCHANGED
    ├── owm_provider.h                # UNCHANGED
    └── openmeteo_provider.h          # NEW: class declaration

src/
├── config.cpp                        # MODIFIED: add Open-Meteo config values
└── provider/
    ├── weather_provider_factory.cpp  # MODIFIED: add #elif for Open-Meteo
    ├── owm_provider.cpp              # UNCHANGED
    └── openmeteo_provider.cpp        # NEW: full implementation (~350-450 lines)
```

**Structure Decision**: Follows the established single-project layout with providers in `include/provider/` and `src/provider/`. No new directories created. Two new files, four modified files.

## Complexity Tracking

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| Retry loop (3 attempts) in fetchData | Matches existing OWM provider behavior; transient network errors on ESP32 are common | Single-attempt approach would reduce reliability on unstable WiFi; next wake cycle is 30 min away |
