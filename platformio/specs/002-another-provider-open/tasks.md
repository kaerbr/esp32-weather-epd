# Tasks: Open-Meteo Weather Provider

**Input**: Design documents from `/specs/002-another-provider-open/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/openmeteo_api_contract.md, quickstart.md

**Tests**: Not requested. Manual validation only (compile, flash, observe per constitution).

**Organization**: Tasks are grouped by user story. US1+US2 are combined (both P1, inseparable — keyless operation is part of URL construction). US3 (max data coverage) and US4 (error handling) are quality attributes woven into the US1 implementation, with explicit verification tasks.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Configuration)

**Purpose**: Add Open-Meteo provider selection macros and configuration variables.

- [ ] T001 [P] Add Open-Meteo provider macros (`USE_PROVIDER_OPENMETEO`, `WEATHER_PROVIDER_OPENMETEO` commented out) and `#ifdef WEATHER_PROVIDER_OPENMETEO` extern block for optional API key in `include/config.h`
- [ ] T002 [P] Add Open-Meteo configuration variables (API key, empty by default) in `src/config.cpp`
- [ ] T003 [P] Update config validation `#if` block at bottom of `include/config.h` to include `WEATHER_PROVIDER_OPENMETEO` in the XOR check

**Checkpoint**: Config compiles with OWM still selected. Open-Meteo macros exist but are commented out.

---

## Phase 2: Foundational (Provider Skeleton)

**Purpose**: Create the provider class skeleton and wire it into the factory. MUST be complete before US1 implementation.

**CRITICAL**: No user story work can begin until this phase is complete.

- [ ] T004 Create `include/provider/openmeteo_provider.h` declaring `OpenMeteoProvider` class inheriting from `WeatherProvider` with `fetchData()` override, guarded by `#ifdef USE_PROVIDER_OPENMETEO`
- [ ] T005 Create `src/provider/openmeteo_provider.cpp` with license header, `#ifdef USE_PROVIDER_OPENMETEO` guard, includes, constructor setting `providerName = "Open-Meteo"`, and stub `fetchData()` returning `HTTP_CODE_OK`
- [ ] T006 Add static WMO code → English description lookup function in `src/provider/openmeteo_provider.cpp` (28 entries per data-model.md WMO Description Lookup Table)
- [ ] T007 Wire Open-Meteo into factory: add `#include "provider/openmeteo_provider.h"` and `#elif defined(WEATHER_PROVIDER_OPENMETEO)` branch in `src/provider/weather_provider_factory.cpp`
- [ ] T008 Add ISRG Root X1 (Let's Encrypt) root CA certificate for `api.open-meteo.com` and `air-quality-api.open-meteo.com` to `include/cert.h`

**Checkpoint**: Switching to `WEATHER_PROVIDER_OPENMETEO` in config.h compiles and runs the stub. Display shows "Open-Meteo" in status bar with no weather data.

---

## Phase 3: User Story 1+2 — Core Weather Fetch & Keyless Operation (Priority: P1) MVP

**Goal**: Fetch current, hourly (48h), and daily (8d) weather data from the Open-Meteo Forecast API and populate `weather_data_t`. Works without an API key; supports optional commercial key.

**Independent Test**: Enable `WEATHER_PROVIDER_OPENMETEO` in config.h with no API key, compile, flash, and verify all weather widgets display data correctly on the e-paper screen.

### Implementation for User Story 1+2

- [ ] T009 [US1] Implement URL construction in `src/provider/openmeteo_provider.cpp`: build forecast API query string with all current/hourly/daily variables from `contracts/openmeteo_api_contract.md`, using `LAT`/`LON` from config, `timezone=auto`, `wind_speed_unit=ms`, `timeformat=unixtime`, `forecast_days=8`, `forecast_hours=48`. Free endpoint by default; commercial endpoint with `apikey` param when configured.
- [ ] T010 [US1] Implement `deserializeForecast()` static function in `src/provider/openmeteo_provider.cpp`: parse metadata (latitude, longitude, timezone, utc_offset_seconds) from JSON response into `weather_data_t` top-level fields. Use ArduinoJson filter document to minimize memory.
- [ ] T011 [US1] Implement current conditions parsing in `deserializeForecast()` in `src/provider/openmeteo_provider.cpp`: map all current variables per `data-model.md` Current Conditions table. Apply conversions: Celsius→Kelvin (+273.15f), snowfall cm→mm (×10), cloud_cover→condition.clouds. Populate condition via WMO lookup. Copy `daily.sunrise[0]` and `daily.sunset[0]` into `current.sunrise` and `current.sunset`.
- [ ] T012 [US1] Implement hourly forecast parsing in `deserializeForecast()` in `src/provider/openmeteo_provider.cpp`: iterate up to `MAX_HOURLY` (48) entries, map all hourly variables per `data-model.md` Hourly Forecast table. Apply conversions: Celsius→Kelvin, snowfall cm→mm, PoP ÷100.0f.
- [ ] T013 [US1] Implement daily forecast parsing in `deserializeForecast()` in `src/provider/openmeteo_provider.cpp`: iterate up to `MAX_DAILY` (8) entries, map all daily variables per `data-model.md` Daily Forecast table including extended variables (mean_relative_humidity_2m, mean_dewpoint_2m, mean_sea_level_pressure, mean_visibility). Apply conversions: Celsius→Kelvin, snowfall cm→mm, PoP ÷100.0f. Leave moonrise/moonset/moon_phase and temp.morn/day/eve/night and feels_like.morn/day/eve/night at sentinel values.
- [ ] T014 [US1] Implement `fetchData()` body in `src/provider/openmeteo_provider.cpp`: set `data.provider_name`, construct forecast URI, execute HTTP GET with retry loop (max 3 attempts), check WiFi status (return `-512 - wifi_status` if disconnected), call `deserializeForecast()` on success (return `-256 - json_error` on parse failure), propagate HTTP error codes on failure. Call `wifi_client.stop()` and `http.end()` after each attempt.

**Checkpoint**: Device fetches weather data from Open-Meteo without API key. All current condition widgets, hourly graph, and daily forecast cards display correctly. Status bar shows "Open-Meteo".

---

## Phase 4: User Story 3 — Maximum Data Coverage (Priority: P2)

**Goal**: Verify and ensure all available Open-Meteo data points are mapped to the `weather_data_t` structure with no gaps.

**Independent Test**: Compare populated fields after fetch against the complete field mapping in `data-model.md`. No available mapping should be missing.

### Implementation for User Story 3

- [ ] T015 [US3] Review `src/provider/openmeteo_provider.cpp` against every row in `data-model.md` tables (Current, Hourly, Daily) and verify each mapping is implemented. Add any missing field assignments. Ensure all unit conversions match the Conversion column.
- [ ] T016 [US3] Verify ArduinoJson filter document in `src/provider/openmeteo_provider.cpp` includes all requested API variables (cross-reference with `contracts/openmeteo_api_contract.md` query parameters). Ensure no variables are filtered out accidentally.

**Checkpoint**: Every field listed as "Direct" or with a conversion in data-model.md is populated. Only fields marked "NOT AVAILABLE" remain at sentinel values.

---

## Phase 5: User Story 4 — Graceful Error Handling (Priority: P2)

**Goal**: Ensure error handling follows the same conventions as other providers, matching the WeatherProvider contract.

**Independent Test**: Simulate errors (disconnect WiFi, use invalid coordinates) and verify correct error codes are returned without hangs or crashes.

### Implementation for User Story 4

- [ ] T017 [US4] Review error handling paths in `fetchData()` in `src/provider/openmeteo_provider.cpp`: verify WiFi status check returns `-512 - wifi_status`, JSON parse errors return `-256 - json_error`, HTTP errors propagate status code, retry loop bounded at 3 attempts, `wifi_client.stop()` and `http.end()` called in all paths.
- [ ] T018 [US4] Add serial debug logging in `src/provider/openmeteo_provider.cpp` matching OWM pattern: log sanitized URL before request (without API key), log HTTP response code after each attempt, log JSON overflow status at `DEBUG_LEVEL >= 1`, print full response at `DEBUG_LEVEL >= 2`.

**Checkpoint**: Error conditions produce correct negative status codes. Serial output shows diagnostic information at configured debug levels.

---

## Phase 6: User Story 5 — Air Quality Data (Priority: P3)

**Goal**: Fetch air quality data from the Open-Meteo Air Quality API and populate pollutant concentration fields.

**Independent Test**: Enable the air quality widget, compile with Open-Meteo provider, flash, and verify AQI and pollutant data display correctly.

### Implementation for User Story 5

- [ ] T019 [US5] Implement `deserializeAirQuality()` static function in `src/provider/openmeteo_provider.cpp`: parse hourly arrays for carbon_monoxide→co, nitrogen_monoxide→no, nitrogen_dioxide→no2, ozone→o3, sulphur_dioxide→so2, pm2_5→pm2_5, pm10→pm10, ammonia→nh3 per `data-model.md` Air Quality table. Iterate up to `MAX_AQ_HOURS` (24). Populate `dt[]` timestamps. Increment `data.num_aq_hours`.
- [ ] T020 [US5] Add air quality API call to `fetchData()` in `src/provider/openmeteo_provider.cpp` after the weather forecast call: construct AQ URI per `contracts/openmeteo_api_contract.md` (past_hours=24, forecast_hours=0), execute HTTP GET with retry loop (max 3 attempts), call `deserializeAirQuality()` on success. If AQ call fails but weather succeeded, return the weather HTTP status (success) — AQ failure is non-fatal.

**Checkpoint**: Air quality widget shows AQI value and description. Pollutant data populates correctly. Weather still displays if AQ API fails.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Final verification across all user stories.

- [ ] T021 Verify full compilation with `-Wall` produces no new warnings when `WEATHER_PROVIDER_OPENMETEO` is enabled in `include/config.h`
- [ ] T022 Verify compilation still works when switching back to `WEATHER_PROVIDER_OWM` in `include/config.h` (no regressions)
- [ ] T023 Review `include/config.h` default template: ensure new Open-Meteo options have sensible defaults and clear comments matching the existing documentation style
- [ ] T024 Verify `data-model.md` "NOT AVAILABLE" fields (moonrise, moonset, moon_phase, temp morn/day/eve/night, feels_like morn/day/eve/night, daily clouds, alerts) remain at sentinel values after fetch

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately. All T001-T003 are parallel.
- **Foundational (Phase 2)**: Depends on Phase 1. T004-T006 are parallel. T007 depends on T004. T008 is independent.
- **US1+US2 (Phase 3)**: Depends on Phase 2. T009 first, then T010-T013 sequentially (building up deserializeForecast), then T014 last (ties everything together).
- **US3 (Phase 4)**: Depends on Phase 3 — review/verification of what was built.
- **US4 (Phase 5)**: Depends on Phase 3 — review/enhancement of error handling in fetchData.
- **US5 (Phase 6)**: Depends on Phase 2 only — can run in parallel with US3/US4 since it adds new code (deserializeAirQuality) and appends to fetchData.
- **Polish (Phase 7)**: Depends on all previous phases.

### User Story Dependencies

- **US1+US2 (P1)**: Depends on Foundational (Phase 2). Core MVP — must be complete first.
- **US3 (P2)**: Depends on US1+US2 — verification pass over existing implementation.
- **US4 (P2)**: Depends on US1+US2 — review/enhancement of error paths.
- **US5 (P3)**: Depends on Foundational only — independent of US3/US4. Adds new code to fetchData after the weather call.

### Within Each User Story

- URL construction → deserialization → fetchData body (sequential within US1)
- Models/parsing before integration
- Core implementation before error handling refinement

### Parallel Opportunities

- **Phase 1**: T001, T002, T003 — all different files
- **Phase 2**: T004, T005, T006, T008 — different files (T007 depends on T004)
- **Phase 4 + Phase 5 + Phase 6**: US3, US4, US5 can proceed in parallel after Phase 3

---

## Parallel Example: Phase 1 (Setup)

```
Task T001: Add provider macros to include/config.h
Task T002: Add provider config to src/config.cpp
Task T003: Update validation block in include/config.h
```

Note: T001 and T003 touch the same file (config.h) but different sections. They can be done sequentially within a single edit session.

## Parallel Example: Phase 2 (Foundational)

```
Task T004: Create include/provider/openmeteo_provider.h
Task T005: Create src/provider/openmeteo_provider.cpp (skeleton)
Task T006: Add WMO lookup function
Task T008: Add certificate to include/cert.h
```

After T004 completes:
```
Task T007: Wire into src/provider/weather_provider_factory.cpp
```

---

## Implementation Strategy

### MVP First (US1+US2 Only)

1. Complete Phase 1: Setup (config macros)
2. Complete Phase 2: Foundational (provider skeleton + factory wiring)
3. Complete Phase 3: US1+US2 (core weather fetch)
4. **STOP and VALIDATE**: Switch to `WEATHER_PROVIDER_OPENMETEO`, compile, flash, verify display
5. Device shows weather data from Open-Meteo without API key — MVP complete

### Incremental Delivery

1. Setup + Foundational → Provider compiles as stub
2. US1+US2 → Weather data displays → **MVP**
3. US3 → Verify max field coverage → Data quality pass
4. US4 → Error handling verified → Robustness pass
5. US5 → Air quality displays → Full feature parity
6. Polish → Clean compilation, no regressions

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- No automated tests — validation is manual per constitution
- Commit after each phase checkpoint
- All temperatures from Open-Meteo are Celsius and MUST be converted to Kelvin (+273.15f) before storing
- All snowfall from Open-Meteo is in cm and MUST be converted to mm (×10)
- All PoP from Open-Meteo is 0-100 and MUST be divided by 100.0f
- Wind speed MUST be requested as m/s via `wind_speed_unit=ms`
- Refer to `data-model.md` for complete field mapping and `contracts/openmeteo_api_contract.md` for exact API query parameters
