# Contract: Open-Meteo API Integration

**Date**: 2026-04-05
**Feature**: 002-open-meteo-provider

## API Endpoints

### Weather Forecast

**Base URL (free)**: `api.open-meteo.com`
**Base URL (commercial)**: `customer-api.open-meteo.com`
**Path**: `/v1/forecast`

**Query parameters**:
```
latitude={LAT}
&longitude={LON}
&current=temperature_2m,relative_humidity_2m,apparent_temperature,dew_point_2m,
  precipitation,rain,showers,snowfall,weather_code,cloud_cover,pressure_msl,
  surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m,visibility,uv_index
&hourly=temperature_2m,relative_humidity_2m,apparent_temperature,dew_point_2m,
  precipitation_probability,rain,snowfall,weather_code,cloud_cover,pressure_msl,
  visibility,wind_speed_10m,wind_direction_10m,wind_gusts_10m,uv_index
&daily=weather_code,temperature_2m_max,temperature_2m_min,apparent_temperature_max,
  apparent_temperature_min,sunrise,sunset,uv_index_max,precipitation_sum,rain_sum,
  snowfall_sum,precipitation_hours,precipitation_probability_max,wind_speed_10m_max,
  wind_gusts_10m_max,wind_direction_10m_dominant,mean_relative_humidity_2m,
  mean_dewpoint_2m,mean_sea_level_pressure,mean_visibility
&timezone=auto
&wind_speed_unit=ms
&timeformat=unixtime
&forecast_days=8
&forecast_hours=48
```

If commercial API key is configured, append: `&apikey={KEY}`

### Air Quality

**Base URL**: `air-quality-api.open-meteo.com`
**Path**: `/v1/air-quality`

**Query parameters**:
```
latitude={LAT}
&longitude={LON}
&hourly=pm10,pm2_5,carbon_monoxide,nitrogen_dioxide,sulphur_dioxide,ozone,
  ammonia,nitrogen_monoxide
&timezone=auto
&timeformat=unixtime
&past_hours=24
&forecast_hours=0
```

## Response Handling

### Success (HTTP 200)
Parse JSON and populate `weather_data_t`. See `data-model.md` for field mapping.

### Error (HTTP 400+)
Return the HTTP status code. Open-Meteo error body: `{"error": true, "reason": "..."}`.

### Network Failure
Return `-512 - wifi_status` per the WeatherProvider contract.

### JSON Parse Failure
Return `-256 - json_error` per the WeatherProvider contract.

## Unit Conversions Required in Provider

| Value | Open-Meteo Unit | Internal Unit | Conversion |
|-------|-----------------|---------------|------------|
| Temperature | °C | Kelvin | +273.15 |
| Snowfall | cm | mm | ×10 |
| PoP | 0-100% | 0.0-1.0 | ÷100 |
| Wind speed | m/s (requested) | m/s | None |
| Pressure | hPa | hPa | None |
| Visibility | meters | meters | None |
| Precipitation | mm | mm | None |

## Certificate Requirements

For `USE_HTTPS_WITH_CERT_VERIF` mode, the following root CA must be present:
- **ISRG Root X1** (Let's Encrypt) — covers both `api.open-meteo.com` and `air-quality-api.open-meteo.com`
