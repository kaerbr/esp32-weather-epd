/* Concrete implementation of WeatherProvider for the Bright Sky API.
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
#include "config.h"
#ifdef USE_PROVIDER_BRIGHTSKY

#pragma once

#include "provider/WeatherProvider.h"
#include <ArduinoJson.h>
#include <WiFiClient.h>

class BrightSkyProvider : public WeatherProvider {
public:
    explicit BrightSkyProvider(WiFiClient& client);
    ~BrightSkyProvider() override;

    bool fetchWeatherData(WeatherData& data) override;

private:
    int fetchForecastData(JsonDocument& doc, const String& date);
    void deserializeForecast(JsonDocument& doc, WeatherData& data);
    void convertUnits(WeatherData& data);

    WiFiClient& wifi_client;
};

#endif
