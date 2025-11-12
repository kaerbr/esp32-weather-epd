/* Concrete implementation of WeatherProvider for the OpenWeatherMap API.
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

#include "provider/WeatherProvider.h"
#include "model/WeatherData.h"
#include <WiFiClient.h>
#include <cstdint>
#include <vector>
#include <ArduinoJson.h>

class OpenWeatherMapProvider : public WeatherProvider {
public:
    explicit OpenWeatherMapProvider(WiFiClient& client);
    ~OpenWeatherMapProvider() override;

    bool fetchWeatherData(WeatherData& data) override;

private:
    int fetchOneCallData(WeatherData& data);
    int fetchAirPollutionData(WeatherData& data);
    void convertUnits(WeatherData& data);
    static DeserializationError deserializeOneCall(WiFiClient& json, WeatherData& data);
    static DeserializationError deserializeAirQuality(WiFiClient& json, WeatherData& data);

    WiFiClient& wifi_client;
};
