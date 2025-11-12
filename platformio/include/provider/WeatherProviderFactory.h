/* Factory for creating weather provider instances.
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
#include <WiFiClient.h>

/**
 * @brief A factory class responsible for creating concrete instances of WeatherProvider.
 *
 * This class decouples the main application from any specific provider implementation.
 * It determines which provider to instantiate based on compile-time configuration.
 */
class WeatherProviderFactory
{
public:
    /**
     * @brief Creates and returns a pointer to a concrete WeatherProvider instance.
     *
     * The selection of which provider to create is handled internally based on
     * configuration settings (e.g., which API keys are defined).
     *
     * @param client A reference to the WiFiClient (or WiFiClientSecure) to be injected
     *               into the provider for networking.
     * @return A pointer to a new WeatherProvider instance, or nullptr if no provider
     *         is configured or available. The caller is responsible for deleting
     *         this pointer when it's no longer needed.
     */
    static WeatherProvider *createProvider(WiFiClient &client);
};
