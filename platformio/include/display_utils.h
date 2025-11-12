/* Display helper utility declarations for esp32-weather-epd.
 * Copyright (C) 2022-2025  Luke Marzen
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

#pragma once

#include <vector>
#include <time.h>
#include "model/WeatherData.h"
#include <WiFi.h>

enum alert_category {
  NOT_FOUND = -1,
  SMOG,
  SMOKE,
  FOG,
  METEOR,
  NUCLEAR,
  BIOHAZARD,
  EARTHQUAKE,
  FIRE,
  HEAT,
  WINTER, 
  TSUNAMI,
  LIGHTNING,
  SANDSTORM,
  FLOOD,
  VOLCANO,
  AIR_QUALITY,
  TORNADO,
  SMALL_CRAFT_ADVISORY,
  GALE_WARNING,
  STORM_WARNING,
  HURRICANE_WARNING,
  HURRICANE,
  DUST,
  STRONG_WIND
};

uint32_t readBatteryVoltage();
uint32_t calcBatPercent(uint32_t v, uint32_t minv, uint32_t maxv);
const uint8_t *getBatBitmap24(uint32_t batPercent);
void getDateStr(String &s, tm *timeInfo);
void getRefreshTimeStr(String &s, bool timeSuccess, tm *timeInfo);
void toTitleCase(String &text);
void truncateExtraAlertInfo(String &text);
void filterAlerts(std::vector<WeatherAlert> &resp, int *ignore_list);
const char *getUVIdesc(unsigned int uvi);
float getAvgConc(const float pollutant[], int hours);
const char *getAQIdesc(int aqi);
const char *getWiFidesc(int rssi);
const uint8_t *getWiFiBitmap16(int rssi);
const uint8_t *getHourlyForecastBitmap32(const HourlyWeather &hourly, const DailyWeather  &today);
const uint8_t *getDailyForecastBitmap64(const DailyWeather &daily);
const uint8_t *getCurrentConditionsBitmap196(const CurrentWeather &current, const DailyWeather   &today);
const uint8_t *getAlertBitmap32(const WeatherAlert &alert);
const uint8_t *getAlertBitmap48(const WeatherAlert &alert);
enum alert_category getAlertCategory(const WeatherAlert &alert);
const uint8_t *getWindBitmap24(int windDeg);
const char *getCompassPointNotation(int windDeg);
const char *getHttpResponsePhrase(int code);
const char *getWifiStatusPhrase(wl_status_t status);
void printHeapUsage();
void disableBuiltinLED();
