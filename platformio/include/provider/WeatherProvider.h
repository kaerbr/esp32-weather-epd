#pragma once

#include "model/WeatherData.h"

/**
 * @brief An abstract base class (interface) for weather data providers.
 *
 * Any class that provides weather data from a specific API (like OpenWeatherMap, DWD, etc.)
 * must inherit from this class and implement its pure virtual functions.
 */
class WeatherProvider
{
public:
    /**
     * @brief Default constructor.
     */
    WeatherProvider() : providerName("") {}

    /**
     * @brief Virtual destructor.
     */
    virtual ~WeatherProvider() {}

    /**
     * @brief Fetches all required weather and air quality data from the provider's API
     *        and populates the given WeatherData struct.
     *
     * This is a pure virtual function that must be implemented by any concrete subclass.
     * The implementation should handle all aspects of the API communication, including
     * making HTTP requests, parsing the response, and mapping the data to the
     * generic WeatherData model.
     *
     * @param data A reference to a WeatherData struct to be filled with the fetched data.
     * @return the http status code of the last successful or first failing http request
     */
    virtual int fetchWeatherData(WeatherData &data) = 0;

    /**
     * @brief The user-friendly name of the weather provider.
     */
    String providerName;

#ifdef USE_HTTP
    static const uint16_t PORT = 80;
#else
    static const uint16_t PORT = 443;
#endif
};
