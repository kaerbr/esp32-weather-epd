# Feature Specification: Open-Meteo Weather Provider

**Feature Branch**: `002-open-meteo-provider`  
**Created**: 2026-04-04  
**Status**: Draft  
**Input**: User description: "add another provider (Open-Meteo) to the app. Fill as many data points as possible. For further information take a deep look in the docs: https://open-meteo.com/en/docs ."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Retrieve Weather Data via Open-Meteo (Priority: P1)

A user selects Open-Meteo as their weather provider in the configuration. The device fetches current conditions, hourly forecasts (48 hours), and daily forecasts (8 days) from the Open-Meteo API and displays them on the e-paper screen identically to how data from any other provider would appear.

**Why this priority**: This is the core value of the feature. Without fetching and displaying weather data, nothing else matters.

**Independent Test**: Can be fully tested by configuring Open-Meteo as the provider, powering the device, and verifying that current conditions, hourly forecast graph, and daily forecast cards all render correctly on the e-paper display.

**Acceptance Scenarios**:

1. **Given** Open-Meteo is configured as the weather provider with valid latitude/longitude, **When** the device wakes and fetches data, **Then** the current weather (temperature, humidity, wind, pressure, conditions, UV index, visibility, dew point, feels-like temperature) is displayed correctly.
2. **Given** Open-Meteo is configured, **When** data is fetched successfully, **Then** hourly forecasts for up to 48 hours are populated with temperature, conditions, wind, humidity, precipitation probability, and UV index.
3. **Given** Open-Meteo is configured, **When** data is fetched successfully, **Then** daily forecasts for up to 8 days are populated with high/low temperatures, conditions, precipitation amounts, wind, humidity, pressure, sunrise/sunset, and UV index.
4. **Given** Open-Meteo is configured, **When** data is fetched successfully, **Then** all WMO weather codes from the API map directly to the application's existing WMO code enum without any lossy translation.

---

### User Story 2 - No API Key Required (Priority: P1)

A user who does not have (or does not want to manage) a paid API key can use Open-Meteo as a free, keyless weather provider for non-commercial personal use.

**Why this priority**: A major advantage of Open-Meteo is that it works without an API key for personal use, lowering the barrier to entry significantly.

**Independent Test**: Can be tested by configuring Open-Meteo without any API key and verifying that the device successfully retrieves and displays weather data.

**Acceptance Scenarios**:

1. **Given** Open-Meteo is selected and no API key is configured, **When** the device fetches data, **Then** the request succeeds and weather data is displayed.
2. **Given** the user has a commercial Open-Meteo API key, **When** they configure it in the settings, **Then** the provider uses the commercial endpoint with the key.

---

### User Story 3 - Fill Maximum Data Points (Priority: P2)

The Open-Meteo provider populates as many fields in the shared weather data structure as possible, maximizing the information available for display widgets.

**Why this priority**: The user explicitly requested filling as many data points as possible. This ensures feature parity with other providers and takes advantage of Open-Meteo's rich data offering.

**Independent Test**: Can be tested by comparing the populated fields in the weather data structure after an Open-Meteo fetch against a reference list of all supported fields, verifying that no available mapping is missing.

**Acceptance Scenarios**:

1. **Given** Open-Meteo returns data, **When** the response is parsed, **Then** all of the following current-condition fields are populated: temperature, feels-like temperature, dew point, UV index, pressure, humidity, visibility, wind speed, wind gust, wind direction, rain amount, snow amount, cloud cover, and weather condition code.
2. **Given** Open-Meteo returns data, **When** the response is parsed, **Then** all of the following hourly fields are populated: temperature, feels-like temperature, dew point, UV index, pressure, humidity, visibility, wind speed, wind gust, wind direction, precipitation probability, rain amount, snow amount, and weather condition code.
3. **Given** Open-Meteo returns data, **When** the response is parsed, **Then** all of the following daily fields are populated: min/max temperature, feels-like min/max, dew point, UV index max, pressure, humidity, visibility, wind speed max, wind gust max, dominant wind direction, precipitation probability max, rain sum, snow sum, sunrise, sunset, and weather condition code.

---

### User Story 4 - Graceful Error Handling (Priority: P2)

When the Open-Meteo API is unreachable or returns an error, the device handles it gracefully using the same error conventions as other providers.

**Why this priority**: Robust error handling is essential for an unattended embedded device that may run for weeks without user interaction.

**Independent Test**: Can be tested by simulating network failures or invalid coordinates and verifying that appropriate error codes are returned and the display shows an error state.

**Acceptance Scenarios**:

1. **Given** the device has no internet connectivity, **When** a fetch is attempted, **Then** the provider returns a WiFi error status code.
2. **Given** invalid coordinates are configured, **When** a fetch is attempted, **Then** the provider returns the HTTP error code from the API.
3. **Given** the API returns malformed JSON, **When** parsing is attempted, **Then** the provider returns a JSON deserialization error code.
4. **Given** a transient network failure, **When** the first fetch attempt fails, **Then** the provider retries up to 3 times before reporting failure.

---

### User Story 5 - Air Quality Data (Priority: P3)

The Open-Meteo provider fetches air quality data from the Open-Meteo Air Quality API and populates the air quality fields in the shared data structure.

**Why this priority**: Air quality is a secondary data source displayed in a dedicated widget. It requires a separate API call and is not part of the core weather forecast.

**Independent Test**: Can be tested by enabling the air quality widget and verifying that AQI values and pollutant concentrations are displayed after an Open-Meteo fetch.

**Acceptance Scenarios**:

1. **Given** Open-Meteo is configured and air quality display is enabled, **When** data is fetched, **Then** hourly AQI index values and individual pollutant concentrations (PM2.5, PM10, O3, NO2, SO2, CO) are populated for up to 24 hours.
2. **Given** the air quality API call fails but the weather API call succeeds, **When** results are processed, **Then** weather data is still displayed and air quality fields remain at their sentinel (missing) values.

---

### Edge Cases

- What happens when Open-Meteo returns fewer hourly data points than the 48-slot array? Remaining slots retain their sentinel (missing) values.
- What happens when the device is located in a region where 15-minute data is unavailable? The provider uses hourly data only, which is globally available.
- What happens when Open-Meteo returns a weather code not present in the application's WMO enum? The condition is mapped to `WMO_UNKNOWN`.
- How does the provider handle the absence of moon phase data? Open-Meteo does not provide moon phase, moonrise, or moonset data. These fields retain their sentinel values and dependent display elements show a placeholder or are skipped.
- What happens when the user configures an API key but the key is invalid? The provider returns the HTTP error code from the commercial endpoint.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST provide an Open-Meteo weather provider that implements the same abstract interface as all other weather providers.
- **FR-002**: System MUST allow the user to select Open-Meteo as the active provider via a compile-time configuration macro.
- **FR-003**: System MUST fetch current weather conditions from the Open-Meteo Forecast API including: temperature, apparent temperature, dew point, relative humidity, pressure (sea-level), surface pressure, wind speed, wind gust, wind direction, precipitation, rain, snowfall, cloud cover, weather code, visibility, and UV index.
- **FR-004**: System MUST fetch hourly forecast data for up to 48 hours including: temperature, apparent temperature, dew point, relative humidity, pressure, wind speed, wind gust, wind direction, precipitation probability, rain, snowfall, cloud cover, weather code, visibility, and UV index.
- **FR-005**: System MUST fetch daily forecast data for up to 8 days including: temperature max/min, apparent temperature max/min, sunrise, sunset, UV index max, precipitation sum, rain sum, snowfall sum, precipitation probability max, wind speed max, wind gust max, dominant wind direction, weather code, and precipitation hours.
- **FR-006**: System MUST use WMO weather interpretation codes from the Open-Meteo API directly, mapping them to the application's existing WMO code enum.
- **FR-007**: System MUST work without an API key for non-commercial use (using the free public endpoint).
- **FR-008**: System MUST support an optional API key for commercial use (using the commercial endpoint).
- **FR-009**: System MUST request data in standard metric units (Celsius, km/h, mm) and rely on the application's existing unit conversion for display.
- **FR-010**: System MUST use automatic timezone resolution so that timestamps align with the device's configured location.
- **FR-011**: System MUST populate the provider name field so it appears in the display status bar.
- **FR-012**: System MUST follow the same error code conventions as other providers: negative codes for WiFi/JSON errors, standard HTTP codes for API errors.
- **FR-013**: System MUST retry failed API requests up to 3 times before returning an error.
- **FR-014**: System MUST register itself in the provider factory so it can be instantiated via the compile-time macro.
- **FR-015**: System MUST fetch air quality data from the Open-Meteo Air Quality API endpoint, populating AQI and pollutant concentration fields (PM2.5, PM10, O3, NO2, SO2, CO) for up to 24 hours.
- **FR-016**: System MUST leave fields at their sentinel values (NAN for floats, INT32_MIN for ints, 0 for timestamps) when a data point is not available from the Open-Meteo API.
- **FR-017**: System MUST construct API requests using the existing latitude and longitude configuration values.

### Key Entities

- **Open-Meteo Forecast Response**: JSON payload containing current, hourly, and daily weather data arrays, plus metadata (latitude, longitude, timezone, UTC offset, elevation).
- **Open-Meteo Air Quality Response**: JSON payload containing hourly air quality index and pollutant concentration arrays.
- **Provider Configuration**: Compile-time macro for provider selection, optional API key, and the shared latitude/longitude/language settings.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: All current-condition display widgets (temperature, humidity, wind, pressure, UV index, visibility, conditions icon) render correctly when using Open-Meteo as the provider.
- **SC-002**: The hourly forecast graph displays 48 hours of data with no gaps when Open-Meteo provides a full response.
- **SC-003**: The daily forecast section displays 8 days of forecasts with high/low temperatures, condition icons, and precipitation information.
- **SC-004**: The device successfully fetches and displays weather data without any API key configured (non-commercial use).
- **SC-005**: All WMO weather codes returned by Open-Meteo (codes 0-99) are correctly mapped to condition icons on the display, with no missing or incorrect icons.
- **SC-006**: When the Open-Meteo API is unreachable, the device reports an error within the same timeout window as other providers, and does not hang or crash.
- **SC-007**: Air quality widget displays AQI and pollutant data when using Open-Meteo, with data no more than 1 hour stale.
- **SC-008**: Data field coverage from Open-Meteo is at least 90% of the fields populated by the existing OpenWeatherMap provider (with documented exceptions for unavailable data such as moon phase).

## Assumptions

- Open-Meteo's free tier remains available for non-commercial personal use without an API key.
- The device has sufficient memory to parse Open-Meteo JSON responses (Open-Meteo responses are structured similarly to other weather APIs and can be filtered with ArduinoJson).
- Open-Meteo's Forecast API returns WMO 4680 standard weather codes, which are the same codes the application already uses internally.
- Moon phase, moonrise, and moonset data are not available from Open-Meteo and will remain at sentinel values. This is an accepted gap.
- Weather alerts are not available from the Open-Meteo free Forecast API. Alert fields will remain empty. This is an accepted gap.
- The Open-Meteo Air Quality API is a separate endpoint from the weather forecast and requires a second HTTP request.
- The application's existing WiFiClient and HTTP/HTTPS infrastructure is sufficient for connecting to Open-Meteo servers.
- The `humidity` and `dew_point` daily fields can be derived from Open-Meteo's daily aggregates (max/mean relative humidity, mean dew point).
- Daily `feels_like` sub-fields (morning, day, evening, night) are not available as discrete daily values from Open-Meteo. Only min/max apparent temperature is available at the daily level. Sub-period values will remain at sentinel values.
- Daily visibility data is available from Open-Meteo's extended daily variables (mean visibility).
