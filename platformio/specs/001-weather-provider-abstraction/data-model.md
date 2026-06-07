# Data Model: Weather Provider Abstraction

**Date**: 2026-04-03
**Feature**: 001-weather-provider-abstraction

## Entity: WMO Weather Code Enum

```
wmo_code_t (uint8_t enum)
├── WMO_CLEAR               = 0
├── WMO_HAZE                = 5
├── WMO_DUST                = 7
├── WMO_MIST                = 10
├── WMO_FOG                 = 45
├── WMO_LIGHTNING            = 13
├── WMO_SQUALL              = 18
├── WMO_TORNADO             = 19
├── WMO_DRIZZLE             = 51
├── WMO_FREEZING_DRIZZLE    = 56
├── WMO_RAIN                = 61
├── WMO_FREEZING_RAIN       = 66
├── WMO_RAIN_SNOW           = 68
├── WMO_SNOW                = 71
├── WMO_SLEET               = 79
├── WMO_HAIL                = 89
├── WMO_THUNDERSTORM        = 95
├── WMO_THUNDERSTORM_HAIL   = 96
├── WMO_SMOKE               = 4
├── WMO_VOLCANIC_ASH        = 200  (project extension)
├── WMO_CLOUDY_FEW          = 201  (project extension)
├── WMO_CLOUDY_SCATTERED    = 202  (project extension)
├── WMO_CLOUDY_BROKEN       = 203  (project extension)
├── WMO_OVERCAST            = 204  (project extension)
└── WMO_UNKNOWN             = 255  (sentinel)
```

Values 0-99 follow the WMO 4677 table. Values 200+ are project-specific
extensions for conditions WMO 4677 does not distinguish (cloud cover
percentages, volcanic ash). The enum integer values are chosen to match
the WMO code where possible, enabling direct assignment for providers
that natively use WMO codes.

## Entity: weather_condition_t

Replaces `owm_weather_t`. Holds the universal weather condition for a
single observation or forecast period.

```
weather_condition_t
├── wmo_code      : wmo_code_t    (WMO 4677 condition code)
├── description   : char[64]      (human-readable, e.g., "light rain")
└── clouds        : int           (cloud cover %, 0-100; INT32_MIN if unavailable)
```

## Entity: weather_current_t

Replaces `owm_current_t`. All fields that exist in the current struct
are preserved. Sentinel values indicate unavailable data.

```
weather_current_t
├── dt            : int64_t       (observation time, Unix UTC)
├── sunrise       : int64_t       (sunrise time, Unix UTC)
├── sunset        : int64_t       (sunset time, Unix UTC)
├── temp          : float         (temperature, Kelvin)
├── feels_like    : float         (apparent temperature, Kelvin)
├── pressure      : int           (sea-level pressure, hPa)
├── humidity      : int           (relative humidity, %)
├── dew_point     : float         (dew point, Kelvin)
├── uvi           : float         (UV index)
├── visibility    : int           (visibility, meters)
├── wind_speed    : float         (wind speed, m/s)
├── wind_gust     : float         (wind gust, m/s)
├── wind_deg      : int           (wind direction, degrees)
├── rain_1h       : float         (rain volume last hour, mm)
├── snow_1h       : float         (snow volume last hour, mm)
└── condition     : weather_condition_t
```

## Entity: weather_hourly_t

Replaces `owm_hourly_t`. Same fields as current plus `pop`.

```
weather_hourly_t
├── dt            : int64_t
├── temp          : float         (Kelvin)
├── feels_like    : float         (Kelvin)
├── pressure      : int           (hPa)
├── humidity      : int           (%)
├── dew_point     : float         (Kelvin)
├── uvi           : float
├── visibility    : int           (meters)
├── wind_speed    : float         (m/s)
├── wind_gust     : float         (m/s)
├── wind_deg      : int           (degrees)
├── pop           : float         (probability of precipitation, 0.0-1.0)
├── rain_1h       : float         (mm)
├── snow_1h       : float         (mm)
└── condition     : weather_condition_t
```

## Entity: weather_temp_t

Replaces `owm_temp_t`.

```
weather_temp_t
├── morn          : float         (Kelvin)
├── day           : float         (Kelvin)
├── eve           : float         (Kelvin)
├── night         : float         (Kelvin)
├── min           : float         (Kelvin)
└── max           : float         (Kelvin)
```

## Entity: weather_feels_like_t

Replaces `owm_feels_like_t`.

```
weather_feels_like_t
├── morn          : float         (Kelvin)
├── day           : float         (Kelvin)
├── eve           : float         (Kelvin)
└── night         : float         (Kelvin)
```

## Entity: weather_daily_t

Replaces `owm_daily_t`. All fields preserved.

```
weather_daily_t
├── dt            : int64_t
├── sunrise       : int64_t
├── sunset        : int64_t
├── moonrise      : int64_t
├── moonset       : int64_t
├── moon_phase    : float         (0.0-1.0)
├── temp          : weather_temp_t
├── feels_like    : weather_feels_like_t
├── pressure      : int           (hPa)
├── humidity      : int           (%)
├── dew_point     : float         (Kelvin)
├── uvi           : float
├── visibility    : int           (meters)
├── wind_speed    : float         (m/s)
├── wind_gust     : float         (m/s)
├── wind_deg      : int           (degrees)
├── pop           : float         (0.0-1.0)
├── rain          : float         (mm)
├── snow          : float         (mm)
└── condition     : weather_condition_t
```

## Entity: weather_alert_t

Replaces `owm_alerts_t`. Uses fixed char arrays instead of String.

```
weather_alert_t
├── event         : char[128]     (alert event name)
├── start         : int64_t       (start time, Unix UTC)
├── end           : int64_t       (end time, Unix UTC)
└── tags          : char[64]      (type of severe weather)
```

## Entity: air_quality_t

Replaces `owm_resp_air_pollution_t`. Holds up to 24 hours of data.

```
air_quality_t
├── aqi           : int[24]       (Air Quality Index per hour, 1-5 scale)
├── co            : float[24]     (CO concentration, ug/m3)
├── no            : float[24]     (NO, ug/m3)
├── no2           : float[24]     (NO2, ug/m3)
├── o3            : float[24]     (O3, ug/m3)
├── so2           : float[24]     (SO2, ug/m3)
├── pm2_5         : float[24]     (PM2.5, ug/m3)
├── pm10          : float[24]     (PM10, ug/m3)
├── nh3           : float[24]     (NH3, ug/m3)
└── dt            : int64_t[24]   (timestamps, Unix UTC)
```

## Entity: weather_data_t (Root)

The top-level provider-agnostic data structure. Replaces the combination
of `owm_resp_onecall_t` + `owm_resp_air_pollution_t`.

```
weather_data_t
├── provider_name  : const char*  (points to static string, e.g., "OpenWeatherMap")
├── lat            : float
├── lon            : float
├── timezone       : char[64]     (IANA timezone name)
├── timezone_offset: int          (seconds from UTC)
├── current        : weather_current_t
├── hourly         : weather_hourly_t[48]
├── daily          : weather_daily_t[8]
├── alerts         : weather_alert_t[8]
├── num_alerts     : int          (actual number of alerts populated, 0-8)
├── air_quality    : air_quality_t
└── num_aq_hours   : int          (actual hours of AQ data populated, 0-24)
```

**Constants**:
- `MAX_HOURLY = 48`
- `MAX_DAILY = 8`
- `MAX_ALERTS = 8`
- `MAX_AQ_HOURS = 24`

## Sentinel Values

| Field Type | Sentinel | Check |
|------------|----------|-------|
| `float` | `NAN` | `std::isnan(val)` |
| `int` | `INT32_MIN` | `val == INT32_MIN` |
| `int64_t` | `0` | `val == 0` |
| `wmo_code_t` | `WMO_UNKNOWN (255)` | `val == WMO_UNKNOWN` |
| `char[]` | `'\0'` | `val[0] == '\0'` |

## Initialization

A helper function `initWeatherData(weather_data_t &data)` sets all
fields to their sentinel values. Providers then overwrite only the
fields they can populate.

## Relationship to Existing Types

| New Type | Replaces | Notes |
|----------|----------|-------|
| `weather_condition_t` | `owm_weather_t` | Drops `id`, `main`, `icon` (OWM-specific); adds `wmo_code` |
| `weather_current_t` | `owm_current_t` | Field-compatible minus `weather.id`/`icon` |
| `weather_hourly_t` | `owm_hourly_t` | Field-compatible minus `weather.id`/`icon` |
| `weather_daily_t` | `owm_daily_t` | Field-compatible minus `weather.id`/`icon` |
| `weather_temp_t` | `owm_temp_t` | Identical fields |
| `weather_feels_like_t` | `owm_owm_feels_like_t` | Identical fields, fixed name |
| `weather_alert_t` | `owm_alerts_t` | char[] instead of String; drops sender_name, description (already filtered) |
| `air_quality_t` | `owm_resp_air_pollution_t` | Drops coord (redundant with root); otherwise same |
| `weather_data_t` | `owm_resp_onecall_t` + `owm_resp_air_pollution_t` | Unified root; fixed array for alerts instead of std::vector |
