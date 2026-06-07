# Data Model: Open-Meteo Weather Provider

## Existing Entities (No Modifications)

The Open-Meteo provider populates the existing `weather_data_t` structure without any schema changes. All entities, fields, and relationships remain identical to the current data model defined in `include/model/weather_data.h`.

## Field Mapping: Open-Meteo API → weather_data_t

### Metadata

| Struct Field | Open-Meteo JSON Path | Conversion |
|---|---|---|
| `provider_name` | — | Static: "Open-Meteo" |
| `lat` | `latitude` | Direct |
| `lon` | `longitude` | Direct |
| `timezone` | `timezone` | String copy |
| `timezone_offset` | `utc_offset_seconds` | Direct |

### Current Conditions (weather_current_t)

| Struct Field | Open-Meteo Variable | Conversion |
|---|---|---|
| `dt` | `current.time` | Direct (unixtime) |
| `sunrise` | `daily.sunrise[0]` | Copy from daily[0] |
| `sunset` | `daily.sunset[0]` | Copy from daily[0] |
| `temp` | `current.temperature_2m` | Celsius → Kelvin (+273.15) |
| `feels_like` | `current.apparent_temperature` | Celsius → Kelvin (+273.15) |
| `pressure` | `current.pressure_msl` | Direct (hPa) |
| `humidity` | `current.relative_humidity_2m` | Direct (%) |
| `dew_point` | `current.dew_point_2m` | Celsius → Kelvin (+273.15) |
| `uvi` | `current.uv_index` | Direct |
| `visibility` | `current.visibility` | Direct (meters) |
| `wind_speed` | `current.wind_speed_10m` | Direct (m/s, requested via API param) |
| `wind_gust` | `current.wind_gusts_10m` | Direct (m/s) |
| `wind_deg` | `current.wind_direction_10m` | Direct (degrees) |
| `rain_1h` | `current.rain` | Direct (mm) |
| `snow_1h` | `current.snowfall` | Direct (cm → mm, multiply by 10) |
| `condition.wmo_code` | `current.weather_code` | Direct cast to wmo_code_t |
| `condition.description` | — | Static WMO lookup table |
| `condition.clouds` | `current.cloud_cover` | Direct (%) |

### Hourly Forecast (weather_hourly_t[48])

| Struct Field | Open-Meteo Variable | Conversion |
|---|---|---|
| `dt` | `hourly.time[i]` | Direct (unixtime) |
| `temp` | `hourly.temperature_2m[i]` | Celsius ��� Kelvin (+273.15) |
| `feels_like` | `hourly.apparent_temperature[i]` | Celsius → Kelvin (+273.15) |
| `pressure` | `hourly.pressure_msl[i]` | Direct (hPa) |
| `humidity` | `hourly.relative_humidity_2m[i]` | Direct (%) |
| `dew_point` | `hourly.dew_point_2m[i]` | Celsius → Kelvin (+273.15) |
| `uvi` | `hourly.uv_index[i]` | Direct |
| `visibility` | `hourly.visibility[i]` | Direct (meters) |
| `wind_speed` | `hourly.wind_speed_10m[i]` | Direct (m/s) |
| `wind_gust` | `hourly.wind_gusts_10m[i]` | Direct (m/s) |
| `wind_deg` | `hourly.wind_direction_10m[i]` | Direct (degrees) |
| `pop` | `hourly.precipitation_probability[i]` | Divide by 100.0 (% → fraction) |
| `rain_1h` | `hourly.rain[i]` | Direct (mm) |
| `snow_1h` | `hourly.snowfall[i]` | cm → mm (multiply by 10) |
| `condition.wmo_code` | `hourly.weather_code[i]` | Direct cast to wmo_code_t |
| `condition.description` | — | Static WMO lookup table |
| `condition.clouds` | `hourly.cloud_cover[i]` | Direct (%) |

### Daily Forecast (weather_daily_t[8])

| Struct Field | Open-Meteo Variable | Conversion |
|---|---|---|
| `dt` | `daily.time[i]` | Direct (unixtime) |
| `sunrise` | `daily.sunrise[i]` | Direct (unixtime) |
| `sunset` | `daily.sunset[i]` | Direct (unixtime) |
| `moonrise` | — | **NOT AVAILABLE** (sentinel: 0) |
| `moonset` | — | **NOT AVAILABLE** (sentinel: 0) |
| `moon_phase` | — | **NOT AVAILABLE** (sentinel: NAN) |
| `temp.min` | `daily.temperature_2m_min[i]` | Celsius → Kelvin (+273.15) |
| `temp.max` | `daily.temperature_2m_max[i]` | Celsius → Kelvin (+273.15) |
| `temp.morn/day/eve/night` | ��� | **NOT AVAILABLE** (sentinel: NAN) |
| `feels_like.morn/day/eve/night` | — | **NOT AVAILABLE** (sentinel: NAN) |
| `pressure` | `daily.mean_sea_level_pressure[i]` | Direct (hPa) |
| `humidity` | `daily.mean_relative_humidity_2m[i]` | Direct (%) |
| `dew_point` | `daily.mean_dewpoint_2m[i]` | Celsius → Kelvin (+273.15) |
| `uvi` | `daily.uv_index_max[i]` | Direct |
| `visibility` | `daily.mean_visibility[i]` | Direct (meters) |
| `wind_speed` | `daily.wind_speed_10m_max[i]` | Direct (m/s) |
| `wind_gust` | `daily.wind_gusts_10m_max[i]` | Direct (m/s) |
| `wind_deg` | `daily.wind_direction_10m_dominant[i]` | Direct (degrees) |
| `pop` | `daily.precipitation_probability_max[i]` | Divide by 100.0 (% → fraction) |
| `rain` | `daily.rain_sum[i]` | Direct (mm) |
| `snow` | `daily.snowfall_sum[i]` | cm → mm (multiply by 10) |
| `condition.wmo_code` | `daily.weather_code[i]` | Direct cast to wmo_code_t |
| `condition.description` | — | Static WMO lookup table |
| `condition.clouds` | — | **NOT AVAILABLE** (sentinel: INT32_MIN) |

### Air Quality (air_quality_t)

Source: `air-quality-api.open-meteo.com/v1/air-quality`

| Struct Field | Open-Meteo Variable | Conversion |
|---|---|---|
| `aqi[i]` | — | Not populated (renderer calculates from raw) |
| `co[i]` | `hourly.carbon_monoxide[i]` | Direct (µg/m³) |
| `no[i]` | `hourly.nitrogen_monoxide[i]` | Direct (µg/m³) |
| `no2[i]` | `hourly.nitrogen_dioxide[i]` | Direct (µg/m³) |
| `o3[i]` | `hourly.ozone[i]` | Direct (µg/m³) |
| `so2[i]` | `hourly.sulphur_dioxide[i]` | Direct (µg/m³) |
| `pm2_5[i]` | `hourly.pm2_5[i]` | Direct (µg/m³) |
| `pm10[i]` | `hourly.pm10[i]` | Direct (µg/m³) |
| `nh3[i]` | `hourly.ammonia[i]` | Direct (µg/m³) |
| `dt[i]` | `hourly.time[i]` | Direct (unixtime) |

### Alerts (weather_alert_t) — NOT AVAILABLE

Open-Meteo does not provide weather alerts. `alerts[]` remains empty, `num_alerts = 0`.

## Snowfall Unit Note

Open-Meteo returns snowfall in **centimeters (cm)**. The existing data structure stores snow in **millimeters (mm)** (matching OWM). The provider must multiply snowfall values by 10 to convert cm → mm.

## WMO Description Lookup Table

Static mapping from WMO code → English description string:

| Code | Description |
|------|-------------|
| 0 | Clear sky |
| 1 | Mainly clear |
| 2 | Partly cloudy |
| 3 | Overcast |
| 45 | Fog |
| 48 | Depositing rime fog |
| 51 | Light drizzle |
| 53 | Moderate drizzle |
| 55 | Dense drizzle |
| 56 | Light freezing drizzle |
| 57 | Dense freezing drizzle |
| 61 | Slight rain |
| 63 | Moderate rain |
| 65 | Heavy rain |
| 66 | Light freezing rain |
| 67 | Heavy freezing rain |
| 71 | Slight snowfall |
| 73 | Moderate snowfall |
| 75 | Heavy snowfall |
| 77 | Snow grains |
| 80 | Slight rain showers |
| 81 | Moderate rain showers |
| 82 | Violent rain showers |
| 85 | Slight snow showers |
| 86 | Heavy snow showers |
| 95 | Thunderstorm |
| 96 | Thunderstorm with slight hail |
| 99 | Thunderstorm with heavy hail |
