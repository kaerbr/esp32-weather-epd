# Gemini Project: esp32-weather-epd

This document provides a comprehensive overview of the `esp32-weather-epd` project, intended to be used as a context file for the Gemini CLI.

## Project Overview

This is a C++ project for an ESP32 microcontroller that displays weather information on an e-paper display (EPD). The project is built using the PlatformIO framework and the Arduino core.

**Key Features:**

*   **Weather Data:** Fetches current weather, hourly forecasts, and daily forecasts from the OpenWeatherMap API.
*   **E-Paper Display:** Renders the weather information on a low-power e-paper display. It supports various display models and color capabilities.
*   **Hardware Sensors:** Can read indoor temperature and humidity from a BME280 or BME680 sensor.
*   **Power Management:** Utilizes the ESP32's deep sleep mode to conserve power, waking up periodically to refresh the weather data. It also includes battery monitoring to protect the battery from over-discharge.
*   **OTA Updates:** Supports over-the-air (OTA) firmware updates using ElegantOTA.
*   **Customizable:** The project is highly customizable through the `include/config.h` and `src/config.cpp` files, allowing users to configure their specific hardware, location, units, and display preferences.

## Building and Running

This project is managed with PlatformIO. The following are the standard PlatformIO commands to build, upload, and monitor the application.  
**Important:** Please do not build the project. The human developer will do that manually.

*   **Build the project:**
    ```bash
    pio run
    ```

*   **Upload the firmware to the ESP32:**
    ```bash
    pio run -t upload
    ```

*   **Monitor the serial output:**
    ```bash
    pio run -t monitor
    ```

*   **Build and Upload for a specific environment:**
    The `platformio.ini` file defines multiple environments (e.g., `dfrobot_firebeetle2_esp32e`, `firebeetle32`). To build for a specific environment, use the `-e` flag:
    ```bash
    pio run -e dfrobot_firebeetle2_esp32e -t upload
    ```

## Development Conventions

*   **Configuration:**
    *   Project features, display settings, units, and other high-level options are configured in `include/config.h`.
    *   Sensitive information (WiFi credentials, API keys) and hardware-specific pin configurations are defined in `src/config.cpp`. This separation helps to keep secrets out of the main configuration file.
*   **Code Style:** The code follows a consistent C++ style. Header files are used for declarations, and implementation files (`.cpp`) contain the definitions.
*   **Modularity:** The code is organized into modules with clear responsibilities (e.g., `renderer`, `WeatherProvider`, `display_utils`).
*   **Power Efficiency:** The application is designed to be power-efficient, with the main logic running in the `setup()` function and the device spending most of its time in deep sleep. The `loop()` function is only used for the OTA server mode.
*   **Extensibility:** The use of a `WeatherProviderFactory` suggests that the project is designed to be extended with other weather data providers in the future.
