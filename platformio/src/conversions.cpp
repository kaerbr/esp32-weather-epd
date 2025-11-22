/* Unit conversion functions for esp32-weather-epd.
 * Copyright (C) 2023  Luke Marzen
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

#include "conversions.h"
#include "config.h" // Include config.h to access unit definitions

#include <cmath>

float kelvin_to_celsius(float kelvin)
{
  return kelvin - 273.15f;
} // end kelvin_to_celsius

float kelvin_to_fahrenheit(float kelvin)
{
  return kelvin * (9.f / 5.f) - 459.67f;
} // end kelvin_to_fahrenheit

float celsius_to_kelvin(float celsius)
{
  return celsius + 273.15f;
} // end celsius_to_kelvin

float celsius_to_fahrenheit(float celsius)
{
  return celsius * (9.f / 5.f) + 32.f;
} // end celsius_to_fahrenheit

float meterspersecond_to_feetpersecond(float meterspersecond)
{
  return meterspersecond * 3.281f;
} // end meterspersecond_to_feetpersecond

float meterspersecond_to_kilometersperhour(float meterspersecond)
{
  return meterspersecond * 3.6f;
} // end meterspersecond_to_kilometersperhour

float meterspersecond_to_milesperhour(float meterspersecond)
{
  return meterspersecond * 2.237f;
} // end meterspersecond_to_milesperhour

float meterspersecond_to_knots(float meterspersecond)
{
  return meterspersecond * 1.944f;
} // end meterspersecond_to_knots

int meterspersecond_to_beaufort(float meterspersecond)
{
  int beaufort = (int) ((powf( 1 / 0.836f, 2.f/3.f)
                         * powf(meterspersecond, 2.f/3.f))
                        + .5f);
  return beaufort > 12 ? 12 : beaufort;
} // end meterspersecond_to_beaufort

float kilometersperhour_to_meterspersecond(float kilometersperhour)
{
  return kilometersperhour / 3.6f;
} // end kilometersperhour_to_meterspersecond

float hectopascals_to_pascals(float hectopascals)
{
  return hectopascals * 100.f;
} // end hectopascals_to_pascals

float hectopascals_to_millimetersofmercury(float hectopascals)
{
  return hectopascals * 0.7501f;
} // end hectopascals_to_millimetersofmercury

float hectopascals_to_inchesofmercury(float hectopascals)
{
  return hectopascals * 0.02953f;
} // end hectopascals_to_inchesofmercury

float hectopascals_to_millibars(float hectopascals)
{
  return hectopascals * 1.f;
} // end hectopascals_to_millibars

float hectopascals_to_atmospheres(float hectopascals)
{
  return hectopascals * 9.869e-4f;
} // end hectopascals_to_atmospheres

float hectopascals_to_gramspersquarecentimeter(float hectopascals)
{
  return hectopascals * 1.02f;
} // end hectopascals_to_gramspersquarecentimeter

float hectopascals_to_poundspersquareinch(float hectopascals)
{
  return hectopascals * 0.0145f;
} // end hectopascals_to_poundspersquareinch

float meters_to_kilometers(float meters)
{
  return meters * 0.001f;
} // end meters_to_kilometers

float meters_to_miles(float meters)
{
  return meters * 6.214e-4f;
} // end meters_to_miles

float meters_to_feet(float meters)
{
  return meters * 3.281f;
} // end meters_to_feet

float millimeters_to_inches(float millimeter)
{
  return millimeter / 25.4f;
} // end milimeters_to_inches

float millimeters_to_centimeters(float millimeter)
{
  return millimeter / 10.0f;
} // end milimeters_to_centimeter

// New conversion functions from metric base units to configured units
float convertTemperature(float celsius) {
  #if defined(UNITS_TEMP_FAHRENHEIT)
    return celsius_to_fahrenheit(celsius);
  #elif defined(UNITS_TEMP_KELVIN)
    return celsius_to_kelvin(celsius);
  #else // UNITS_TEMP_CELSIUS
    return celsius;
  #endif
}

float convertWindSpeed(float metersPerSecond) {
  #if defined(UNITS_SPEED_KILOMETERSPERHOUR)
    return meterspersecond_to_kilometersperhour(metersPerSecond);
  #elif defined(UNITS_SPEED_MILESPERHOUR)
    return meterspersecond_to_milesperhour(metersPerSecond);
  #elif defined(UNITS_SPEED_FEETPERSECOND)
    return meterspersecond_to_feetpersecond(metersPerSecond);
  #elif defined(UNITS_SPEED_KNOTS)
    return meterspersecond_to_knots(metersPerSecond);
  #elif defined(UNITS_SPEED_BEAUFORT)
    return (float)meterspersecond_to_beaufort(metersPerSecond);
  #else // UNITS_SPEED_METERSPERSECOND
    return metersPerSecond;
  #endif
}

float convertPressure(float hectopascals) {
  #if defined(UNITS_PRES_PASCALS)
    return hectopascals_to_pascals(hectopascals);
  #elif defined(UNITS_PRES_MILLIMETERSOFMERCURY)
    return hectopascals_to_millimetersofmercury(hectopascals);
  #elif defined(UNITS_PRES_INCHESOFMERCURY)
    return hectopascals_to_inchesofmercury(hectopascals);
  #elif defined(UNITS_PRES_MILLIBARS)
    return hectopascals_to_millibars(hectopascals);
  #elif defined(UNITS_PRES_ATMOSPHERES)
    return hectopascals_to_atmospheres(hectopascals);
  #elif defined(UNITS_PRES_GRAMSPERSQUARECENTIMETER)
    return hectopascals_to_gramspersquarecentimeter(hectopascals);
  #elif defined(UNITS_PRES_POUNDSPERSQUAREINCH)
    return hectopascals_to_poundspersquareinch(hectopascals);
  #else // UNITS_PRES_HECTOPASCALS
    return hectopascals;
  #endif
}

float convertVisibility(float meters) {
  #if defined(UNITS_DIST_KILOMETERS)
    return meters_to_kilometers(meters);
  #elif defined(UNITS_DIST_MILES)
    return meters_to_miles(meters);
  #else
    return meters; // Default to meters if no other distance unit is defined
  #endif
}

float convertPrecipitation(float millimeters) {
  // Prioritize hourly precipitation units, then daily, then default to millimeters
  #if defined(UNITS_HOURLY_PRECIP_INCHES) || defined(UNITS_DAILY_PRECIP_INCHES)
    return millimeters_to_inches(millimeters);
  #elif defined(UNITS_HOURLY_PRECIP_CENTIMETERS) || defined(UNITS_DAILY_PRECIP_CENTIMETERS)
    return millimeters_to_centimeters(millimeters);
  #else // UNITS_HOURLY_PRECIP_MILLIMETERS or UNITS_DAILY_PRECIP_MILLIMETERS
    return millimeters;
  #endif
}

// Function to convert all units in WeatherData object
void convertWeatherDataUnits(WeatherData &data)
{
    // Current weather
    data.current.temp = convertTemperature(data.current.temp);
    data.current.feels_like = convertTemperature(data.current.feels_like);
    data.current.pressure = convertPressure(data.current.pressure);
    data.current.visibility = convertVisibility(data.current.visibility);
    data.current.wind_speed = convertWindSpeed(data.current.wind_speed);
    data.current.wind_gust = convertWindSpeed(data.current.wind_gust);

    // Hourly forecast
    for (int i = 0; i < MAX_HOURLY_FORECASTS; ++i)
    {
        data.hourly[i].temp = convertTemperature(data.hourly[i].temp);
        data.hourly[i].rain_1h = convertPrecipitation(data.hourly[i].rain_1h);
        data.hourly[i].snow_1h = convertPrecipitation(data.hourly[i].snow_1h);
        data.hourly[i].wind_speed = convertWindSpeed(data.hourly[i].wind_speed);
        data.hourly[i].wind_gust = convertWindSpeed(data.hourly[i].wind_gust);
    }

    // Daily forecast
    for (int i = 0; i < MAX_DAILY_FORECASTS; ++i)
    {
        data.daily[i].temp_min = convertTemperature(data.daily[i].temp_min);
        data.daily[i].temp_max = convertTemperature(data.daily[i].temp_max);
        data.daily[i].rain = convertPrecipitation(data.daily[i].rain);
        data.daily[i].snow = convertPrecipitation(data.daily[i].snow);
        data.daily[i].wind_speed = convertWindSpeed(data.daily[i].wind_speed);
        data.daily[i].wind_gust = convertWindSpeed(data.daily[i].wind_gust);
    }
}

