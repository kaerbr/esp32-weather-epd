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
  wmo_code_t wmo_code;
  char       description[64];
  int        clouds;
} weather_condition_t;

typedef struct weather_current
{
  int64_t             dt;
  int64_t             sunrise;
  int64_t             sunset;
  float               temp;
  float               feels_like;
  int                 pressure;
  int                 humidity;
  float               dew_point;
  float               uvi;
  int                 visibility;
  float               wind_speed;
  float               wind_gust;
  int                 wind_deg;
  float               rain_1h;
  float               snow_1h;
  weather_condition_t condition;
} weather_current_t;

typedef struct weather_hourly
{
  int64_t             dt;
  float               temp;
  float               feels_like;
  int                 pressure;
  int                 humidity;
  float               dew_point;
  float               uvi;
  int                 visibility;
  float               wind_speed;
  float               wind_gust;
  int                 wind_deg;
  float               pop;
  float               rain_1h;
  float               snow_1h;
  weather_condition_t condition;
} weather_hourly_t;

typedef struct weather_temp
{
  float morn;
  float day;
  float eve;
  float night;
  float min;
  float max;
} weather_temp_t;

typedef struct weather_feels_like
{
  float morn;
  float day;
  float eve;
  float night;
} weather_feels_like_t;

typedef struct weather_daily
{
  int64_t              dt;
  int64_t              sunrise;
  int64_t              sunset;
  int64_t              moonrise;
  int64_t              moonset;
  float                moon_phase;
  weather_temp_t       temp;
  weather_feels_like_t feels_like;
  int                  pressure;
  int                  humidity;
  float                dew_point;
  float                uvi;
  int                  visibility;
  float                wind_speed;
  float                wind_gust;
  int                  wind_deg;
  float                pop;
  float                rain;
  float                snow;
  weather_condition_t  condition;
} weather_daily_t;

typedef struct weather_alert
{
  char    event[128];
  int64_t start;
  int64_t end;
  char    tags[64];
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
} air_quality_t;

typedef struct weather_data
{
  const char         *provider_name;
  float               lat;
  float               lon;
  char                timezone[64];
  int                 timezone_offset;
  weather_current_t   current;
  weather_hourly_t    hourly[MAX_HOURLY];
  weather_daily_t     daily[MAX_DAILY];
  weather_alert_t     alerts[MAX_ALERTS];
  int                 num_alerts;
  air_quality_t       air_quality;
  int                 num_aq_hours;
} weather_data_t;

void initWeatherData(weather_data_t &data);

inline bool isSentinelFloat(float val)     { return std::isnan(val); }
inline bool isSentinelInt(int val)         { return val == INT32_MIN; }
inline bool isSentinelTimestamp(int64_t val){ return val == 0; }

#endif
