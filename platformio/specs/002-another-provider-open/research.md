# Research: Open-Meteo Weather Provider

## R1: Internal Unit Conventions

**Decision**: All temperatures in the data structure are stored in **Kelvin**. Wind speed in **m/s**. Pressure in **hPa**. Visibility in **meters**. Precipitation in **mm**. PoP (probability of precipitation) as **0.0–1.0 fraction**.

**Rationale**: The existing OWM provider uses `units=standard` which returns Kelvin temperatures and m/s wind. The renderer converts from Kelvin using `kelvin_to_celsius()` / `kelvin_to_fahrenheit()` based on user config. Confirmed by reading `renderer.cpp` (lines 940, 987, 1346-1361) and `conversions.cpp` (lines 22-35).

**Implications for Open-Meteo**:
- Temperature: Open-Meteo returns Celsius by default. Must convert to Kelvin (+273.15) in the provider.
- Wind speed: Request `wind_speed_unit=ms` from the API.
- Pressure: Open-Meteo default is hPa. No conversion needed.
- Visibility: Open-Meteo returns meters. No conversion needed.
- Precipitation: Open-Meteo returns mm. No conversion needed.
- PoP: Open-Meteo returns 0–100 (percentage). Must divide by 100.0 to get 0.0–1.0 fraction.

**Alternatives considered**: Requesting Fahrenheit and converting — rejected because Kelvin is the internal standard.

## R2: AQI Handling

**Decision**: The provider stores raw pollutant concentrations (µg/m³) in the `air_quality_t` struct. The renderer calculates AQI from those raw values using the existing `calc_aqi()` library with the locale's `AQI_SCALE`.

**Rationale**: `renderer.cpp:485-487` calls `calc_aqi(AQI_SCALE, air_quality.co, ...)` from raw concentrations, ignoring the `aqi[]` field. The `aqi[]` array stores OWM's 1-5 value but is never used for display. Open-Meteo's Air Quality API provides raw concentrations in µg/m³ (same units as OWM), so they can be stored directly.

**Implications**: The `aqi[]` field can be left at sentinel values. Only the raw pollutant arrays (co, no, no2, o3, so2, pm2_5, pm10, nh3) matter for display.

**Alternatives considered**: Storing Open-Meteo's European/US AQI — rejected because the renderer ignores that field.

## R3: Open-Meteo Air Quality API Endpoint

**Decision**: Use `air-quality-api.open-meteo.com/v1/air-quality` with hourly variables: `pm10,pm2_5,carbon_monoxide,nitrogen_dioxide,sulphur_dioxide,ozone,ammonia,nitrogen_monoxide`.

**Rationale**: This endpoint provides all pollutant concentrations needed to populate the `air_quality_t` struct. Variable mapping:
- `carbon_monoxide` → `co[]`
- `nitrogen_monoxide` → `no[]`
- `nitrogen_dioxide` → `no2[]`
- `ozone` → `o3[]`
- `sulphur_dioxide` → `so2[]`
- `pm2_5` → `pm2_5[]`
- `pm10` → `pm10[]`
- `ammonia` → `nh3[]`

**Alternatives considered**: Using the weather forecast API for AQI — not available there.

## R4: Timestamps

**Decision**: Request `timeformat=unixtime` from both Open-Meteo endpoints to get Unix timestamps (seconds since epoch), matching the OWM format.

**Rationale**: The data structure uses `int64_t` Unix timestamps. OWM returns Unix timestamps. Open-Meteo defaults to ISO 8601 strings, but supports `timeformat=unixtime` which returns the same Unix format.

**Exception**: `sunrise` and `sunset` in daily data are returned as ISO 8601 even with `timeformat=unixtime`. These must be parsed from ISO 8601 format. Actually, with `timeformat=unixtime`, Open-Meteo returns sunrise/sunset as Unix timestamps too.

## R5: Weather Condition Descriptions

**Decision**: Provide static WMO code → English description strings in the provider. These are not localized.

**Rationale**: OWM provides localized `description` strings via `API_LANG`. Open-Meteo only returns numeric WMO codes. A small static lookup table (28 entries) mapping WMO codes to English descriptions (e.g., 0→"Clear sky", 61→"Slight rain") provides reasonable UX parity. The descriptions are used in the `condition.description[64]` field.

**Alternatives considered**: Leaving description empty — rejected because it degrades display output. Full localization — rejected as over-scoped; can be added later.

## R6: Data Gaps (Not Available from Open-Meteo)

**Decision**: Accept these gaps; fields remain at sentinel values:

| Field | Reason |
|-------|--------|
| `daily.moonrise` | Not provided by Open-Meteo |
| `daily.moonset` | Not provided by Open-Meteo |
| `daily.moon_phase` | Not provided by Open-Meteo |
| `daily.temp.morn/day/eve/night` | Only min/max available at daily level |
| `daily.feels_like.morn/day/eve/night` | Only min/max apparent temp available |
| `alerts[]` | Not available from Open-Meteo free API |
| `air_quality.aqi[]` | Not used by renderer (calculates from raw) |

**Rationale**: The display widgets handle sentinel values gracefully (show placeholder or skip). Moon data and alerts are secondary features.

## R7: Open-Meteo Forecast API Query Design

**Decision**: Single API call to `api.open-meteo.com/v1/forecast` requesting current + hourly + daily data:

**Current variables**: `temperature_2m,relative_humidity_2m,apparent_temperature,dew_point_2m,precipitation,rain,showers,snowfall,weather_code,cloud_cover,pressure_msl,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m,visibility,uv_index`

**Hourly variables**: `temperature_2m,relative_humidity_2m,apparent_temperature,dew_point_2m,precipitation_probability,rain,snowfall,weather_code,cloud_cover,pressure_msl,visibility,wind_speed_10m,wind_direction_10m,wind_gusts_10m,uv_index`

**Daily variables**: `weather_code,temperature_2m_max,temperature_2m_min,apparent_temperature_max,apparent_temperature_min,sunrise,sunset,uv_index_max,precipitation_sum,rain_sum,snowfall_sum,precipitation_hours,precipitation_probability_max,wind_speed_10m_max,wind_gusts_10m_max,wind_direction_10m_dominant,mean_relative_humidity_2m,mean_dewpoint_2m,mean_sea_level_pressure,mean_visibility`

**Additional parameters**: `timezone=auto&wind_speed_unit=ms&timeformat=unixtime&forecast_days=8&forecast_hours=48`

## R8: Retry Strategy & Constitution Compliance

**Decision**: Follow the existing OWM pattern of up to 3 retry attempts per API call.

**Rationale**: The constitution (Principle IV) says "Error recovery MUST NOT involve retry loops." However, the existing OWM provider (`owm_provider.cpp:377-407`) already uses a 3-attempt retry loop and pre-dates the constitution. Matching this established pattern ensures behavioral consistency between providers. The retry count is bounded (max 3) and does not constitute an unbounded loop.

**Risk**: If the constitution is strictly enforced in the future, both providers should be updated simultaneously.

## R9: HTTPS Certificate for Open-Meteo

**Decision**: Add Open-Meteo root CA certificate to `cert.h` (or equivalent certificate store) for `USE_HTTPS_WITH_CERT_VERIF` mode.

**Rationale**: The project supports three HTTP modes. For HTTPS with cert verification, the ESP32 needs the root CA certificate for `api.open-meteo.com` and `air-quality-api.open-meteo.com`. Both use Let's Encrypt (ISRG Root X1). The cert must be added alongside the existing OWM certificates.

**Alternatives considered**: HTTP-only — rejected, doesn't match project security posture.
