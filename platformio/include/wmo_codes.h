/* WMO 4677 weather condition code definitions for esp32-weather-epd.
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

#ifndef __WMO_CODES_H__
#define __WMO_CODES_H__

#include <cstdint>

// Grouped WMO 4677 weather condition categories.
// Values 0-99 follow the WMO 4677 table where possible.
// Values 200+ are project-specific extensions.
enum wmo_code_t : uint8_t
{
  WMO_CLEAR              = 0,
  WMO_SMOKE              = 4,
  WMO_HAZE               = 5,
  WMO_DUST               = 7,
  WMO_MIST               = 10,
  WMO_LIGHTNING           = 13,
  WMO_SQUALL             = 18,
  WMO_TORNADO            = 19,
  WMO_FOG                = 45,
  WMO_DRIZZLE            = 51,
  WMO_FREEZING_DRIZZLE   = 56,
  WMO_RAIN               = 61,
  WMO_FREEZING_RAIN      = 66,
  WMO_RAIN_SNOW          = 68,
  WMO_SNOW               = 71,
  WMO_SLEET              = 79,
  WMO_HAIL               = 89,
  WMO_THUNDERSTORM       = 95,
  WMO_THUNDERSTORM_HAIL  = 96,
  WMO_VOLCANIC_ASH       = 200,
  WMO_CLOUDY_FEW         = 201,
  WMO_CLOUDY_SCATTERED   = 202,
  WMO_CLOUDY_BROKEN      = 203,
  WMO_OVERCAST           = 204,
  WMO_UNKNOWN            = 255
};

bool isDaytime(int64_t dt, int64_t sunrise, int64_t sunset);

#endif
