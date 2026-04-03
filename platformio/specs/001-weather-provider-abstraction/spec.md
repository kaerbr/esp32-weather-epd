# Feature Specification: Weather Provider Abstraction

**Feature Branch**: `001-weather-provider-abstraction`
**Created**: 2026-04-03
**Status**: Draft
**Input**: Decouple data fetching from UI rendering to create a plug-and-play weather provider architecture

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Existing OpenWeatherMap Users Experience No Change (Priority: P1)

A user who currently runs esp32-weather-epd with OpenWeatherMap updates
their firmware to the new version. Without changing any configuration,
the device continues to wake, fetch weather data from OpenWeatherMap,
render the display, and enter deep sleep exactly as before.

**Why this priority**: This is the foundation. The refactor MUST NOT
break the existing user experience. Every other story depends on the
core data pipeline working through the new abstraction layer.

**Independent Test**: Flash the updated firmware with an unmodified
config file. The device renders weather data identically to the
previous version, including all current conditions, hourly graph,
daily forecast, alerts, and air quality.

**Acceptance Scenarios**:

1. **Given** a device running the previous firmware with a valid
   OpenWeatherMap API key, **When** the user flashes the new firmware
   without changing config, **Then** the display renders all weather
   data with no visible differences (temperature, forecast, icons,
   hourly graph, alerts, air quality, status bar).
2. **Given** the new firmware with OpenWeatherMap selected, **When**
   the device completes a wake cycle, **Then** the total active time
   (WiFi on to deep sleep entry) does not increase by more than 5%
   compared to the previous firmware.
3. **Given** the new firmware, **When** the OpenWeatherMap API returns
   an error, **Then** the device displays the appropriate error icon
   and enters deep sleep, identical to current behavior.

---

### User Story 2 - Config-Driven Provider Selection (Priority: P2)

A user who wants to switch weather providers changes a single
configuration value in the config files. After recompiling and
flashing, the device fetches data from the newly selected provider
instead of OpenWeatherMap.

**Why this priority**: This is the primary user-facing value of the
feature. Config-driven selection is what makes the architecture
plug-and-play for end users.

**Independent Test**: Change the weather provider setting in config to
a different provider. Compile, flash, and verify the device fetches
from the new provider's API and renders weather data on the display.

**Acceptance Scenarios**:

1. **Given** the config file with the weather provider set to
   "OpenWeatherMap" (default), **When** the user compiles and flashes,
   **Then** data is fetched from the OpenWeatherMap API.
2. **Given** a valid alternative provider configured, **When** the
   user compiles and flashes, **Then** data is fetched from that
   provider's API endpoints instead.
3. **Given** a misspelled or unsupported provider value in config,
   **When** the user attempts to compile, **Then** compilation fails
   with a clear error message indicating the invalid provider.

---

### User Story 3 - Provider Attribution on Display (Priority: P3)

A user glances at their E-Paper display and can see which weather
provider supplied the data currently shown on screen.

**Why this priority**: Attribution is important for user trust and for
meeting potential API terms of service. It builds on the provider
selection from US2.

**Independent Test**: With any provider configured, verify the
provider name appears on the display in a readable location.

**Acceptance Scenarios**:

1. **Given** OpenWeatherMap is the active provider, **When** the
   display renders, **Then** "OpenWeatherMap" (or an abbreviated
   form) is visible in the status bar at the bottom of the display.
2. **Given** a different provider is active, **When** the display
   renders, **Then** that provider's name is shown in the status bar
   instead.

---

### User Story 4 - Graceful Handling of Missing Metrics (Priority: P4)

A future weather provider does not supply all metrics that
OpenWeatherMap provides (e.g., no UV index, no air quality, no
sunrise/sunset times). The display renders all available data and
shows exactly `--` for any metric the provider could not supply.

**Why this priority**: Different providers have different data
coverage. The display MUST NOT crash or show garbage when data is
absent. This is essential for the plug-and-play promise.

**Independent Test**: Configure a provider that omits UV index and
air quality data. Verify those widget positions display `--` while
all other data renders normally.

**Acceptance Scenarios**:

1. **Given** a provider that does not supply UV index, **When** the
   display renders the current conditions panel, **Then** the UV
   index widget shows `--` as its value.
2. **Given** a provider that does not supply air quality data,
   **When** the display renders, **Then** the air quality widget
   shows `--` and no crash or watchdog reset occurs.
3. **Given** a provider that supplies all metrics, **When** the
   display renders, **Then** all widgets show real data with no
   placeholder hyphens.

---

### User Story 5 - Universal Weather Condition Codes (Priority: P5)

The display shows weather condition icons (thunderstorm, rain, snow,
fog, clear, etc.) using a universal standard instead of
OpenWeatherMap's proprietary condition codes. Each provider translates
its native codes into the universal standard during data fetching,
and the renderer selects icons based solely on the universal codes.

**Why this priority**: This decouples icon rendering from any single
provider's code system. Without this, every new provider would need
to modify the rendering logic.

**Independent Test**: Verify that the icon selection logic operates
on universal weather codes rather than OpenWeatherMap IDs. Compare
the icons displayed by the old firmware vs. the new firmware for
a range of weather conditions and confirm visual equivalence.

**Acceptance Scenarios**:

1. **Given** the OpenWeatherMap provider returns condition code 200
   (thunderstorm with light rain), **When** the fetcher processes
   the response, **Then** the universal data object contains the
   equivalent WMO 4677 code for thunderstorm.
2. **Given** a universal weather code for "heavy rain" in the data
   object, **When** the renderer selects an icon, **Then** it
   chooses the rain icon without referencing any provider-specific
   code.
3. **Given** a provider returns a condition code that has no exact
   WMO equivalent, **When** the fetcher translates it, **Then** it
   maps to the closest reasonable WMO code rather than leaving it
   unmapped.

---

### Edge Cases

- What happens when a provider returns an empty or malformed JSON
  response? The device MUST display an error icon and enter deep
  sleep without crashing.
- What happens when a provider returns valid JSON but with unexpected
  field names? The fetcher MUST treat missing fields as unavailable
  data (sentinel values), not crash.
- What happens when the universal weather code mapping encounters an
  unknown provider-specific code? The fetcher MUST map it to a
  generic "unknown" condition code, and the renderer MUST display
  a fallback icon.
- What happens when all optional metrics are missing from a provider?
  The display MUST still render the layout correctly with `--` in
  every optional widget, and the device MUST enter deep sleep
  normally.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST define a provider-agnostic data
  structure that can hold current conditions, hourly forecasts, daily
  forecasts, alerts, and air quality data from any weather provider.
- **FR-002**: The system MUST use sentinel values (e.g., NAN for
  floats, INT_MIN for integers, 0 for timestamps) in the
  provider-agnostic data structure to indicate metrics not supplied
  by the active provider.
- **FR-003**: The rendering logic MUST display exactly `--` (two
  hyphens) for any numeric metric whose value equals its sentinel.
- **FR-004**: The active weather provider MUST be selectable via a
  single compile-time configuration option in config.h.
- **FR-005**: An invalid or unsupported provider selection MUST
  produce a compile-time error with a descriptive message.
- **FR-006**: Each weather provider MUST translate its native weather
  condition codes into WMO 4677 standard codes before populating the
  provider-agnostic data structure.
- **FR-007**: The icon selection logic MUST operate exclusively on
  WMO 4677 codes and day/night context, never on provider-specific
  condition codes or icon strings. Day/night MUST be derived by
  comparing the observation timestamp against sunrise/sunset
  timestamps in the weather data, not from provider-specific fields.
- **FR-008**: The E-Paper display MUST show the name of the active
  weather provider in the status bar, alongside the existing refresh
  time and battery/WiFi indicators.
- **FR-009**: The OpenWeatherMap provider MUST be the default and
  MUST produce identical display output to the pre-refactor firmware
  for the same API response data.
- **FR-010**: Each provider implementation MUST handle its own API
  endpoint construction, HTTP request, JSON deserialization, and
  data mapping into the provider-agnostic structure. This includes
  air quality data; each provider is responsible for fetching air
  quality as part of its data pipeline (providers that do not offer
  air quality leave those fields at sentinel values).
- **FR-011**: The provider-agnostic data structure MUST NOT use the
  Arduino String class. All string fields MUST use fixed-size char
  arrays or const char* with static storage.
- **FR-012**: The total number of HTTP requests per wake cycle MUST
  NOT increase compared to the current implementation for any given
  provider.

### Key Entities

- **Weather Data Object**: A provider-agnostic structure holding
  current conditions, hourly forecasts (up to 48 entries), daily
  forecasts (up to 8 entries), weather alerts, and air quality data.
  Uses sentinel values for missing metrics. Contains a provider name
  field for display attribution.
- **Weather Condition Code**: A WMO 4677 integer code representing a
  universal weather condition (e.g., thunderstorm, drizzle, rain,
  snow, fog, clear, cloudy). Each provider maps its native codes to
  this standard.
- **Weather Provider**: A module responsible for fetching data from
  a specific API, deserializing the response, translating condition
  codes to WMO 4677, and populating the Weather Data Object.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A community contributor can add a new weather provider
  by creating a single new fetcher module and a code mapping, without
  modifying the renderer, display utilities, or main orchestration
  logic.
- **SC-002**: Switching between providers requires changing exactly
  one configuration value and recompiling. No other file edits are
  needed.
- **SC-003**: The device's active time per wake cycle (from WiFi-on
  to deep-sleep entry) does not increase by more than 5% compared to
  the pre-refactor firmware when using the OpenWeatherMap provider.
- **SC-004**: All current display elements (current conditions,
  hourly graph, daily forecast, alerts, air quality, status bar)
  render identically to the pre-refactor firmware when using
  OpenWeatherMap with the same API response data.
- **SC-005**: When a provider omits a metric, the corresponding
  display widget shows `--` and no crash, watchdog reset, or
  rendering artifact occurs.

## Clarifications

### Session 2026-04-03

- Q: Should air quality be part of each weather provider's responsibility or a separate module? → A: Each weather provider is responsible for fetching air quality as part of its data pipeline.
- Q: Where should the provider attribution be displayed? → A: In the status bar at the bottom, alongside refresh time and battery/WiFi indicators.
- Q: How should day/night be determined for icon selection without provider-specific icon strings? → A: Derived from sunrise/sunset timestamps in the weather data (compare observation dt against sunrise/sunset).

## Assumptions

- OpenWeatherMap remains the default and only initially shipped
  provider. Additional providers will be contributed by the community
  after the architecture is in place.
- The WMO 4677 code table provides sufficient granularity to
  represent weather conditions from all major weather APIs. Where
  exact mappings are ambiguous, the closest reasonable code is
  acceptable.
- Air quality is the responsibility of each weather provider, not a
  separate module. The provider-agnostic structure accommodates a
  common subset (AQI index + key pollutant concentrations). Providers
  that do not offer air quality leave those fields at sentinel values.
- The existing E-Paper layout and widget positions remain unchanged.
  Only the data source and attribution text change.
- Provider API endpoints, authentication methods, and response
  formats vary. Each provider module is responsible for its own
  HTTP configuration (endpoints, headers, ports, certificates).
