# Tasks: Weather Provider Abstraction

**Input**: Design documents from `/specs/001-weather-provider-abstraction/`
**Prerequisites**: plan.md (required), spec.md (required), research.md, data-model.md, contracts/

**Tests**: Not applicable. Validation is manual (compile, flash, observe) per constitution.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story. US5 (WMO codes) is architecturally inseparable from the foundational types and US1 renderer migration, so its tasks are distributed across Phase 2 (foundational) and Phase 3 (US1).

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- Headers: `include/` at repository root
- Sources: `src/` at repository root
- Provider headers: `include/providers/`
- Provider sources: `src/providers/`

---

## Phase 1: Setup

**Purpose**: Create directory structure for new provider modules

- [ ] T001 Create `include/providers/` and `src/providers/` directories for provider implementations

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Define the universal data types, WMO code standard, and abstract provider interface that ALL user stories depend on. No user story work can begin until this phase is complete.

**CRITICAL**: These files are the foundation of the entire architecture.

- [ ] T002 [P] Create `include/wmo_codes.h` defining the `wmo_code_t` enum with all WMO 4677 grouped categories (WMO_CLEAR, WMO_HAZE, WMO_DUST, WMO_MIST, WMO_FOG, WMO_LIGHTNING, WMO_SQUALL, WMO_TORNADO, WMO_DRIZZLE, WMO_FREEZING_DRIZZLE, WMO_RAIN, WMO_FREEZING_RAIN, WMO_RAIN_SNOW, WMO_SNOW, WMO_SLEET, WMO_HAIL, WMO_THUNDERSTORM, WMO_THUNDERSTORM_HAIL, WMO_SMOKE, WMO_VOLCANIC_ASH, WMO_CLOUDY_FEW, WMO_CLOUDY_SCATTERED, WMO_CLOUDY_BROKEN, WMO_OVERCAST, WMO_UNKNOWN) plus `isDaytime(int64_t dt, int64_t sunrise, int64_t sunset)` declaration per data-model.md and research.md R2/R4
- [ ] T003 [P] Create `include/weather_data.h` defining all universal structs: `weather_condition_t`, `weather_current_t`, `weather_hourly_t`, `weather_temp_t`, `weather_feels_like_t`, `weather_daily_t`, `weather_alert_t`, `air_quality_t`, and root `weather_data_t` with constants (MAX_HOURLY=48, MAX_DAILY=8, MAX_ALERTS=8, MAX_AQ_HOURS=24) per data-model.md. All string fields MUST use fixed-size `char[]` arrays. Include `initWeatherData(weather_data_t &data)` declaration. Include sentinel check helpers: `isSentinelFloat(float)`, `isSentinelInt(int)`, `isSentinelTimestamp(int64_t)`.
- [ ] T004 [P] Create `include/weather_provider.h` defining the `WeatherProvider` abstract base class with pure virtual methods `getName()` and `fetchData()` per contracts/weather_provider_interface.md. Use `#ifdef USE_HTTP` to conditionally declare the WiFiClient vs WiFiClientSecure parameter variant.
- [ ] T005 Create `src/wmo_codes.cpp` implementing `isDaytime()` function (compares dt against sunrise/sunset timestamps) per research.md R4
- [ ] T006 Create `src/weather_data.cpp` implementing `initWeatherData()` that sets all fields to sentinel values (NAN for float, INT32_MIN for int, 0 for int64_t, WMO_UNKNOWN for wmo_code_t, '\0' for char arrays) per data-model.md sentinel table

**Checkpoint**: Universal types and interfaces defined. Provider and renderer work can now begin.

---

## Phase 3: User Story 1 - Existing OWM Users Experience No Change (Priority: P1) + User Story 5 - Universal Weather Condition Codes (Priority: P5)

**Goal**: Extract all OpenWeatherMap-specific logic into `OpenWeatherMapProvider`, migrate renderer and display utilities from `owm_*` types to `weather_data_t` types, remap icon selection from OWM condition codes to WMO codes, and replace `isDay(icon)` with `isDaytime(dt, sunrise, sunset)`. The device MUST produce identical display output to the pre-refactor firmware.

**Independent Test**: Flash firmware with unmodified config. Compare display output to previous firmware — all weather data, icons, hourly graph, daily forecast, alerts, air quality, and status bar MUST render identically.

### Implementation for User Story 1 + User Story 5

- [ ] T007 [P] [US1] Create `include/providers/owm_provider.h` declaring `OpenWeatherMapProvider` class inheriting from `WeatherProvider` with `getName()` and `fetchData()` method signatures
- [ ] T008 [P] [US1] Create `src/providers/owm_provider.cpp` implementing `OpenWeatherMapProvider::fetchData()` by extracting and adapting logic from current `src/client_utils.cpp` (`getOWMonecall` + `getOWMairpollution`) and `src/api_response.cpp` (`deserializeOneCall` + `deserializeAirQuality`). Map all JSON fields to `weather_data_t` struct. Implement `owmIdToWmo(int owmId)` static helper that translates OWM condition codes (200-804) to `wmo_code_t` per research.md R3 mapping table. Populate `data.provider_name` with `"OpenWeatherMap"`.
- [ ] T009 [US1] Update `include/display_utils.h` — change all function signatures from `owm_*` types to `weather_data_t` equivalents: `getHourlyForecastBitmap32(const weather_hourly_t&, const weather_daily_t&)`, `getDailyForecastBitmap64(const weather_daily_t&)`, `getCurrentConditionsBitmap196(const weather_current_t&, const weather_daily_t&)`, `getAlertBitmap32/48(const weather_alert_t&)`, `getAlertCategory(const weather_alert_t&)`, `filterAlerts(weather_alert_t*, int, int*)`, `getAQI(const air_quality_t&)`, `getMoonPhaseBitmap48(const weather_daily_t&)`, `getMoonPhaseStr(const weather_daily_t&)`
- [ ] T010 [US1] Update `src/display_utils.cpp` — change `getConditionsBitmap<>()` template to accept `wmo_code_t` instead of OWM `int id`. Rewrite the switch statement to match on `wmo_code_t` enum values per research.md R2 icon mapping table. Update `getHourlyForecastBitmap32` to use `isDaytime(hourly.dt, today.sunrise, today.sunset)` instead of `isDay(hourly.weather.icon)`. Update `getDailyForecastBitmap64` and `getCurrentConditionsBitmap196` similarly. Update `filterAlerts`, `getAlertCategory`, `getAlertBitmap32/48` to use `weather_alert_t` with `char[]` fields. Update `getAQI` to accept `air_quality_t`. Update `getMoonPhaseBitmap48` and `getMoonPhaseStr` to accept `weather_daily_t`.
- [ ] T011 [US1] Update `include/renderer.h` — change all function signatures from `owm_*` types to `weather_data_t` equivalents: `drawCurrentConditions(const weather_current_t&, const weather_daily_t&, const air_quality_t&, float, float)`, `drawForecast(const weather_daily_t*, tm)`, `drawAlerts(weather_alert_t*, int, const String&, const String&)`, `drawOutlookGraph(const weather_hourly_t*, const weather_daily_t*, tm)`, all `drawCurrent*` widget functions.
- [ ] T012 [US1] Update `src/renderer.cpp` — change `drawCurrentConditions()` and all individual widget functions (`drawCurrentSunrise`, `drawCurrentSunset`, `drawCurrentWind`, `drawCurrentHumidity`, `drawCurrentUVI`, `drawCurrentPressure`, `drawCurrentVisibility`, `drawCurrentAirQuality`, `drawCurrentMoonrise`, `drawCurrentMoonset`, `drawCurrentMoonphase`, `drawCurrentDewpoint`) to accept `weather_current_t` / `weather_daily_t` / `air_quality_t` instead of `owm_*` types. Update `drawForecast()`, `drawAlerts()`, and `drawOutlookGraph()` similarly. Replace all `owm_alerts_t` String field accesses with `char[]` equivalents.
- [ ] T013 [US1] Update `src/client_utils.cpp` — remove `getOWMonecall()` and `getOWMairpollution()` functions (logic moved to OWM provider). Keep `startWiFi()`, `killWiFi()`, `waitForSNTPSync()`, `printLocalTime()`, `printHeapUsage()`, and `getHttpResponsePhrase()`. Remove `#include "api_response.h"` and `#include "renderer.h"` if no longer needed.
- [ ] T014 [US1] Update `include/client_utils.h` — remove `getOWMonecall()` and `getOWMairpollution()` declarations. Remove `#include "api_response.h"`. Keep WiFi/NTP/utility declarations.
- [ ] T015 [US1] Update `src/main.cpp` — replace `static owm_resp_onecall_t owm_onecall` and `static owm_resp_air_pollution_t owm_air_pollution` with `static weather_data_t weatherData`. Add `#include "weather_data.h"`, `#include "weather_provider.h"`, and `#include "providers/owm_provider.h"`. Directly instantiate `static OpenWeatherMapProvider weatherProvider` (no `#ifdef` guard yet — the config-driven conditional is added later in T017, Phase 4). Replace the two separate `getOWMonecall`/`getOWMairpollution` call blocks with a single `weatherProvider.fetchData(client, weatherData)` call (preceded by `initWeatherData(weatherData)`). Update all renderer calls to pass `weatherData.*` fields instead of `owm_onecall.*` / `owm_air_pollution`. Update `drawAlerts()` call to pass `weatherData.alerts` array + `weatherData.num_alerts` instead of `owm_onecall.alerts` vector.
- [ ] T016 [US1] Compile full project with `pio run` and fix any type errors, missing includes, or signature mismatches until build succeeds cleanly with `-Wall`

**Checkpoint**: At this point, the device should produce identical display output to the pre-refactor firmware using the OWM provider through the new abstraction layer. US1 and US5 acceptance scenarios are satisfied.

---

## Phase 4: User Story 2 - Config-Driven Provider Selection (Priority: P2)

**Goal**: Make the active weather provider selectable via a single `#define` in `config.h` with compile-time validation.

**Independent Test**: Change the provider macro in `config.h` to an invalid value — compilation MUST fail with a descriptive error. Change back to `WEATHER_PROVIDER_OWM` — compilation MUST succeed.

### Implementation for User Story 2

- [ ] T017 [US2] Update `include/config.h` — add `WEATHER PROVIDER` section (above the existing config validation block) with `#define WEATHER_PROVIDER_OWM` as default. Add compile-time validation using XOR pattern: `#if !(defined(WEATHER_PROVIDER_OWM))` → `#error Invalid configuration. Exactly one weather provider must be selected.` Guard all OWM-specific extern declarations (`OWM_APIKEY`, `OWM_ENDPOINT`, `OWM_ONECALL_VERSION`) with `#ifdef WEATHER_PROVIDER_OWM`. Then update `src/main.cpp` to wrap the direct OWM provider include/instantiation (from T015) in `#if defined(WEATHER_PROVIDER_OWM)` conditional with `#else #error` fallback for missing provider.
- [ ] T018 [US2] Update `src/config.cpp` — guard all OWM-specific variable definitions (`OWM_APIKEY`, `OWM_ENDPOINT`, `OWM_ONECALL_VERSION`) with `#ifdef WEATHER_PROVIDER_OWM`
- [ ] T019 [US2] Compile and verify: build succeeds with `WEATHER_PROVIDER_OWM` defined. Comment it out — build MUST fail with clear error message.

**Checkpoint**: Provider is selectable via a single macro. US2 acceptance scenarios are satisfied.

---

## Phase 5: User Story 3 - Provider Attribution on Display (Priority: P3)

**Goal**: Display the active provider's name in the status bar alongside refresh time and battery/WiFi indicators.

**Independent Test**: Flash firmware and visually verify the provider name ("OpenWeatherMap") appears in the status bar at the bottom of the E-Paper display.

### Implementation for User Story 3

- [ ] T020 [US3] Update `drawStatusBar()` signature in `include/renderer.h` to accept an additional `const char *providerName` parameter
- [ ] T021 [US3] Update `drawStatusBar()` in `src/renderer.cpp` to render `providerName` in the status bar area (left-aligned or after refresh time string, using `FONT_7pt8b` for consistency with existing status bar text)
- [ ] T022 [US3] Update `src/main.cpp` to pass `weatherProvider.getName()` to `drawStatusBar()` call

**Checkpoint**: Provider name is visible in the status bar. US3 acceptance scenarios are satisfied.

---

## Phase 6: User Story 4 - Graceful Handling of Missing Metrics (Priority: P4)

**Goal**: Any metric at its sentinel value renders as `--` on the display. No crash or rendering artifact occurs when a provider omits data.

**Independent Test**: Temporarily modify the OWM provider to skip populating UV index and air quality fields. Flash firmware and verify those widgets show `--` while all other data renders normally. Revert the modification.

### Implementation for User Story 4

- [ ] T023 [US4] Update `drawCurrentSunrise()` and `drawCurrentSunset()` in `src/renderer.cpp` to check `isSentinelTimestamp(current.sunrise)` / `isSentinelTimestamp(current.sunset)` and render `"--"` if true
- [ ] T024 [P] [US4] Update `drawCurrentWind()` in `src/renderer.cpp` to check `isSentinelFloat(current.wind_speed)` and render `"--"` if true
- [ ] T025 [P] [US4] Update `drawCurrentHumidity()` in `src/renderer.cpp` to check `isSentinelInt(current.humidity)` and render `"--"` if true
- [ ] T026 [P] [US4] Update `drawCurrentUVI()` in `src/renderer.cpp` to check `isSentinelFloat(current.uvi)` and render `"--"` if true
- [ ] T027 [P] [US4] Update `drawCurrentPressure()` in `src/renderer.cpp` to check `isSentinelInt(current.pressure)` and render `"--"` if true
- [ ] T028 [P] [US4] Update `drawCurrentVisibility()` in `src/renderer.cpp` to check `isSentinelInt(current.visibility)` and render `"--"` if true
- [ ] T029 [P] [US4] Update `drawCurrentDewpoint()` in `src/renderer.cpp` to check `isSentinelFloat(current.dew_point)` and render `"--"` if true
- [ ] T030 [US4] Update `drawCurrentAirQuality()` in `src/renderer.cpp` to check if air quality data is unavailable (`num_aq_hours == 0` or sentinel AQI values) and render `"--"` if true
- [ ] T031 [US4] Update `drawForecast()` in `src/renderer.cpp` to check sentinel values on `daily[i].temp.max`, `daily[i].temp.min`, and precipitation before rendering; display `"--"` for any unavailable forecast metric
- [ ] T032 [US4] Update `drawOutlookGraph()` in `src/renderer.cpp` to skip sentinel hourly data points in the temperature line and precipitation bars gracefully (do not plot sentinel values)
- [ ] T033 [US4] Compile and verify clean build with `-Wall`

**Checkpoint**: All widgets handle missing data gracefully. US4 acceptance scenarios are satisfied.

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Cleanup, deprecation, and final validation

- [ ] T034 Remove or mark `include/api_response.h` and `src/api_response.cpp` as deprecated — verify no remaining `#include "api_response.h"` references exist outside of `src/providers/owm_provider.cpp` (which may still use it internally or have absorbed its logic)
- [ ] T035 Final compile with `pio run` — verify zero warnings with `-Wall`
- [ ] T036 Verify `sizeof(weather_data_t)` does not exceed available SRAM budget by logging it at startup under `DEBUG_LEVEL >= 1` in `src/main.cpp`
- [ ] T037 Document pre-refactor wake cycle active time (from serial output "Awake for X.XXXs") and compare against post-refactor time to verify SC-003 (<=5% increase). The existing `beginDeepSleep()` in `src/main.cpp` already prints this value — no code change needed, just record and compare.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup — BLOCKS all user stories
- **US1+US5 (Phase 3)**: Depends on Foundational — core refactor
- **US2 (Phase 4)**: Depends on Phase 3 (needs provider in main.cpp)
- **US3 (Phase 5)**: Depends on Phase 3 (needs weatherProvider.getName())
- **US4 (Phase 6)**: Depends on Phase 3 (needs sentinel helpers and universal types in renderer)
- **Polish (Phase 7)**: Depends on all previous phases

### User Story Dependencies

- **US1+US5 (P1+P5)**: MUST complete first — all other stories depend on the universal type migration
- **US2 (P2)**: Can start after Phase 3 — independent of US3 and US4
- **US3 (P3)**: Can start after Phase 3 — independent of US2 and US4
- **US4 (P4)**: Can start after Phase 3 — independent of US2 and US3

### Within Each Phase

- Foundational: T002, T003, T004 are parallel (different files). T005 depends on T002. T006 depends on T003.
- US1: T007, T008 are parallel (provider files). T009-T014 can proceed in parallel (different files) once T007/T008 exist. T015 (main.cpp) depends on all prior US1 tasks. T016 (compile) depends on T015.
- US4: T024-T029 are parallel (different widget functions in same file, but independent changes)

### Parallel Opportunities

- Phase 2: T002, T003, T004 can all run in parallel
- Phase 3: T007, T008 can run in parallel; T009-T014 can run in parallel
- Phase 4-6: US2, US3, US4 can run in parallel after Phase 3 completes

---

## Implementation Strategy

### MVP First (US1 + US5 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL — blocks all stories)
3. Complete Phase 3: US1 + US5
4. **STOP and VALIDATE**: Compile, flash, compare display to pre-refactor
5. If display matches → MVP achieved

### Incremental Delivery

1. Setup + Foundational → types ready
2. US1+US5 → core refactor complete, display identical → **MVP!**
3. US2 → provider selectable via config
4. US3 → provider name in status bar
5. US4 → graceful missing data handling
6. Polish → cleanup deprecated files, final validation

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- US5 (WMO codes) is embedded in Phase 2 (foundational types) and Phase 3 (US1 icon remapping) because it is architecturally inseparable from the core refactor
- No test tasks generated — project uses manual validation per constitution
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
