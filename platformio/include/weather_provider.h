/* Weather provider abstract base class for esp32-weather-epd.
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

#ifndef __WEATHER_PROVIDER_H__
#define __WEATHER_PROVIDER_H__

#include "weather_data.h"
#include "config.h"

#ifdef USE_HTTP
  #include <WiFiClient.h>
#else
  #include <WiFiClientSecure.h>
#endif

class WeatherProvider
{
public:
  virtual ~WeatherProvider() = default;
  virtual const char* getName() const = 0;
#ifdef USE_HTTP
  virtual int fetchData(WiFiClient &client, weather_data_t &data) = 0;
#else
  virtual int fetchData(WiFiClientSecure &client, weather_data_t &data) = 0;
#endif
};

#endif
