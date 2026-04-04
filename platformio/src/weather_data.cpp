/* Universal weather data initialization for esp32-weather-epd.
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

#include <cmath>
#include <climits>
#include <cstring>
#include "weather_data.h"

static void initCondition(weather_condition_t &c)
{
  c.wmo_code = WMO_UNKNOWN;
  c.description[0] = '\0';
  c.clouds = INT32_MIN;
}

static void initCurrent(weather_current_t &c)
{
  c.dt         = 0;
  c.sunrise    = 0;
  c.sunset     = 0;
  c.temp       = NAN;
  c.feels_like = NAN;
  c.pressure   = INT32_MIN;
  c.humidity   = INT32_MIN;
  c.dew_point  = NAN;
  c.uvi        = NAN;
  c.visibility = INT32_MIN;
  c.wind_speed = NAN;
  c.wind_gust  = NAN;
  c.wind_deg   = INT32_MIN;
  c.rain_1h    = NAN;
  c.snow_1h    = NAN;
  initCondition(c.condition);
}

static void initHourly(weather_hourly_t &h)
{
  h.dt         = 0;
  h.temp       = NAN;
  h.feels_like = NAN;
  h.pressure   = INT32_MIN;
  h.humidity   = INT32_MIN;
  h.dew_point  = NAN;
  h.uvi        = NAN;
  h.visibility = INT32_MIN;
  h.wind_speed = NAN;
  h.wind_gust  = NAN;
  h.wind_deg   = INT32_MIN;
  h.pop        = NAN;
  h.rain_1h    = NAN;
  h.snow_1h    = NAN;
  initCondition(h.condition);
}

static void initTemp(weather_temp_t &t)
{
  t.morn  = NAN;
  t.day   = NAN;
  t.eve   = NAN;
  t.night = NAN;
  t.min   = NAN;
  t.max   = NAN;
}

static void initFeelsLike(weather_feels_like_t &f)
{
  f.morn  = NAN;
  f.day   = NAN;
  f.eve   = NAN;
  f.night = NAN;
}

static void initDaily(weather_daily_t &d)
{
  d.dt         = 0;
  d.sunrise    = 0;
  d.sunset     = 0;
  d.moonrise   = 0;
  d.moonset    = 0;
  d.moon_phase = NAN;
  initTemp(d.temp);
  initFeelsLike(d.feels_like);
  d.pressure   = INT32_MIN;
  d.humidity   = INT32_MIN;
  d.dew_point  = NAN;
  d.uvi        = NAN;
  d.visibility = INT32_MIN;
  d.wind_speed = NAN;
  d.wind_gust  = NAN;
  d.wind_deg   = INT32_MIN;
  d.pop        = NAN;
  d.rain       = NAN;
  d.snow       = NAN;
  initCondition(d.condition);
}

static void initAlert(weather_alert_t &a)
{
  a.event[0] = '\0';
  a.start    = 0;
  a.end      = 0;
  a.tags[0]  = '\0';
}

static void initAirQuality(air_quality_t &aq)
{
  for (int i = 0; i < MAX_AQ_HOURS; ++i)
  {
    aq.aqi[i]   = INT32_MIN;
    aq.co[i]    = NAN;
    aq.no[i]    = NAN;
    aq.no2[i]   = NAN;
    aq.o3[i]    = NAN;
    aq.so2[i]   = NAN;
    aq.pm2_5[i] = NAN;
    aq.pm10[i]  = NAN;
    aq.nh3[i]   = NAN;
    aq.dt[i]    = 0;
  }
}

void initWeatherData(weather_data_t &data)
{
  data.provider_name    = "";
  data.lat              = NAN;
  data.lon              = NAN;
  data.timezone[0]      = '\0';
  data.timezone_offset  = INT32_MIN;

  initCurrent(data.current);

  for (int i = 0; i < MAX_HOURLY; ++i)
  {
    initHourly(data.hourly[i]);
  }
  for (int i = 0; i < MAX_DAILY; ++i)
  {
    initDaily(data.daily[i]);
  }
  for (int i = 0; i < MAX_ALERTS; ++i)
  {
    initAlert(data.alerts[i]);
  }
  data.num_alerts = 0;

  initAirQuality(data.air_quality);
  data.num_aq_hours = 0;
}
