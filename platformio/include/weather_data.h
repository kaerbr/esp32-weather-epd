/* Universal weather data structure definitions for esp32-weather-epd.
 * Copyright (C) 2022-2026  Luke Marzen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __WEATHER_DATA_H__
#define __WEATHER_DATA_H__

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <climits>
#include "wmo_codes.h"

constexpr int MAX_HOURLY   = 48;
constexpr int MAX_DAILY    = 8;
constexpr int MAX_ALERTS   = 8;
constexpr int MAX_AQ_HOURS = 24;

typedef struct weather_condition
{
  wmo_code_t wmo_code   = WMO_UNKNOWN;
  char       description[64] = {};
  int        clouds      = INT32_MIN;
} weather_condition_t;

typedef struct weather_current
{
  int64_t             dt         = 0;
  int64_t             sunrise    = 0;
  int64_t             sunset     = 0;
  float               temp       = NAN;
  float               feels_like = NAN;
  int                 pressure   = INT32_MIN;
  int                 humidity   = INT32_MIN;
  float               dew_point  = NAN;
  float               uvi        = NAN;
  int                 visibility = INT32_MIN;
  float               wind_speed = NAN;
  float               wind_gust  = NAN;
  int                 wind_deg   = INT32_MIN;
  float               rain_1h    = NAN;
  float               snow_1h    = NAN;
  weather_condition_t condition  = {};
} weather_current_t;

typedef struct weather_hourly
{
  int64_t             dt         = 0;
  float               temp       = NAN;
  float               feels_like = NAN;
  int                 pressure   = INT32_MIN;
  int                 humidity   = INT32_MIN;
  float               dew_point  = NAN;
  float               uvi        = NAN;
  int                 visibility = INT32_MIN;
  float               wind_speed = NAN;
  float               wind_gust  = NAN;
  int                 wind_deg   = INT32_MIN;
  float               pop        = NAN;
  float               rain_1h    = NAN;
  float               snow_1h    = NAN;
  weather_condition_t condition  = {};
} weather_hourly_t;

typedef struct weather_temp
{
  float morn  = NAN;
  float day   = NAN;
  float eve   = NAN;
  float night = NAN;
  float min   = NAN;
  float max   = NAN;
} weather_temp_t;

typedef struct weather_feels_like
{
  float morn  = NAN;
  float day   = NAN;
  float eve   = NAN;
  float night = NAN;
} weather_feels_like_t;

typedef struct weather_daily
{
  int64_t              dt         = 0;
  int64_t              sunrise    = 0;
  int64_t              sunset     = 0;
  int64_t              moonrise   = 0;
  int64_t              moonset    = 0;
  float                moon_phase = NAN;
  weather_temp_t       temp       = {};
  weather_feels_like_t feels_like = {};
  int                  pressure   = INT32_MIN;
  int                  humidity   = INT32_MIN;
  float                dew_point  = NAN;
  float                uvi        = NAN;
  int                  visibility = INT32_MIN;
  float                wind_speed = NAN;
  float                wind_gust  = NAN;
  int                  wind_deg   = INT32_MIN;
  float                pop        = NAN;
  float                rain       = NAN;
  float                snow       = NAN;
  weather_condition_t  condition  = {};
} weather_daily_t;

typedef struct weather_alert
{
  char    event[128] = {};
  int64_t start      = 0;
  int64_t end        = 0;
  char    tags[64]   = {};
} weather_alert_t;

typedef struct air_quality
{
  int     aqi[MAX_AQ_HOURS];
  float   co[MAX_AQ_HOURS];
  float   no[MAX_AQ_HOURS];
  float   no2[MAX_AQ_HOURS];
  float   o3[MAX_AQ_HOURS];
  float   so2[MAX_AQ_HOURS];
  float   pm2_5[MAX_AQ_HOURS];
  float   pm10[MAX_AQ_HOURS];
  float   nh3[MAX_AQ_HOURS];
  int64_t dt[MAX_AQ_HOURS];

  air_quality()
  {
    std::fill(aqi,   aqi   + MAX_AQ_HOURS, INT32_MIN);
    std::fill(co,    co    + MAX_AQ_HOURS, NAN);
    std::fill(no,    no    + MAX_AQ_HOURS, NAN);
    std::fill(no2,   no2   + MAX_AQ_HOURS, NAN);
    std::fill(o3,    o3    + MAX_AQ_HOURS, NAN);
    std::fill(so2,   so2   + MAX_AQ_HOURS, NAN);
    std::fill(pm2_5, pm2_5 + MAX_AQ_HOURS, NAN);
    std::fill(pm10,  pm10  + MAX_AQ_HOURS, NAN);
    std::fill(nh3,   nh3   + MAX_AQ_HOURS, NAN);
    std::fill(dt,    dt    + MAX_AQ_HOURS, (int64_t)0);
  }
} air_quality_t;

typedef struct weather_data
{
  const char         *provider_name   = nullptr;
  float               lat             = NAN;
  float               lon             = NAN;
  char                timezone[64]    = {};
  int                 timezone_offset = INT32_MIN;
  weather_current_t   current         = {};
  weather_hourly_t    hourly[MAX_HOURLY] = {};
  weather_daily_t     daily[MAX_DAILY]   = {};
  weather_alert_t     alerts[MAX_ALERTS] = {};
  int                 num_alerts      = 0;
  air_quality_t       air_quality     = {};
  int                 num_aq_hours    = 0;
} weather_data_t;

inline bool isSentinelFloat(float val)     { return std::isnan(val); }
inline bool isSentinelInt(int val)         { return val == INT32_MIN; }
inline bool isSentinelTimestamp(int64_t val){ return val == 0; }

#endif
