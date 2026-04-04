#pragma once

#include <cstdint>

/**
 * @brief WMO 4680 (wawa) weather codes
 * source: https://www.nodc.noaa.gov/archive/arc0021/0002199/1.1/data/0-data/HTML/WMO-CODE/WMO4677.HTM
 * source: https://open-meteo.com/en/docs#weather_variable_documentation
 * Naming convention: CATEGORY_MODIFIER_INTENSITY
 * Values 100+ is reserved for project-specific extensions.
 */
enum wmo_code_t : uint8_t
{
  // Clear / Clouds
  WMO_CLEAR_SKY                       = 0,
  WMO_MAINLY_CLEAR                    = 1,
  WMO_PARTLY_CLOUDY                   = 2,
  WMO_OVERCAST                        = 3,

  // Fog
  WMO_FOG                             = 45,
  WMO_FOG_DEPOSITING_RIME             = 48,

  // Drizzle
  WMO_DRIZZLE_LIGHT                   = 51,
  WMO_DRIZZLE_MODERATE                = 53,
  WMO_DRIZZLE_DENSE                   = 55,
  WMO_DRIZZLE_FREEZING_LIGHT          = 56,
  WMO_DRIZZLE_FREEZING_DENSE          = 57,

  // Rain
  WMO_RAIN_SLIGHT                     = 61,
  WMO_RAIN_MODERATE                   = 63,
  WMO_RAIN_HEAVY                      = 65,
  WMO_RAIN_FREEZING_LIGHT             = 66,
  WMO_RAIN_FREEZING_HEAVY             = 67,

  // Snow
  WMO_SNOW_SLIGHT                     = 71,
  WMO_SNOW_MODERATE                   = 73,
  WMO_SNOW_HEAVY                      = 75,
  WMO_SNOW_GRAINS                     = 77,

  // Showers
  WMO_SHOWERS_RAIN_SLIGHT             = 80,
  WMO_SHOWERS_RAIN_MODERATE           = 81,
  WMO_SHOWERS_RAIN_VIOLENT            = 82,
  WMO_SHOWERS_SNOW_SLIGHT             = 85,
  WMO_SHOWERS_SNOW_HEAVY              = 86,

  // Thunderstorms
  WMO_THUNDERSTORM_SLIGHT_OR_MODERATE = 95,
  WMO_THUNDERSTORM_HAIL_SLIGHT        = 96,
  WMO_THUNDERSTORM_HAIL_HEAVY         = 99,

  WMO_UNKNOWN                         = UINT8_MAX
};
