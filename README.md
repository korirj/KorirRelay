# Korir Relay

## Overview

The **Korir Relay** project is an IoT application designed to control multiple relays via MQTT, enabling remote management of connected devices. It utilizes an ESP8266 microcontroller to connect to WiFi, allowing configuration and control through a web interface. Did I mention that it can be connected to both Home assistant and Openhab through MQTT?. Local connection can be implemented through the push buttons.

## Features

- **WiFi and MQTT Configuration:** Easily configure WiFi and MQTT settings through a web interface.
- **Multiple Relay Control:** Supports up to four relays, each controllable through MQTT messages.
- **Auto-Discovery:** Automatically publishes relay configurations to Home Assistant for easy integration.
- **SPIFFS for Configuration Storage:** Utilizes SPIFFS to store configuration settings persistently.

## External Libraries

This project requires the following libraries:

- [ESP8266WiFi](https://github.com/esp8266/Arduino)
- [Ticker](https://github.com/matthijskooijman/arduino-ticker)
- [ESPUI](https://github.com/Makuna/ESPUI)
- [FS](https://github.com/esp8266/Arduino)
- [LittleFS](https://github.com/lorol/arduino-littlefs)
- [DNSServer](https://github.com/esp8266/Arduino)
- [MQTT](https://github.com/256dpi/arduino-mqtt)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [PCF8574](https://github.com/xreef/PCF8574_library)

## Hardware Requirements

- ESP8266 microcontroller (e.g., NodeMCU)
- PCF8574 I/O expander
- Relay module (compatible with the chosen hardware)
- Push buttons (optional for manual control)

## Setup Instructions

1. **Install Dependencies:**
   Ensure all required libraries are installed in your Arduino IDE.

2. **Configure the Microcontroller:**
   - Connect the PCF8574 to the ESP8266:
     - SDA to GPIO4 (D2)
     - SCL to GPIO5 (D1)
   - Connect the relay module and any push buttons.

3. **Upload Code:**
   Load the provided code into your Arduino IDE and upload it to the ESP8266.

4. **Configuration:**
   - If the device is not configured, it will create a hotspot for configuration.
   - Connect to the hotspot and navigate to the web interface to set up WiFi and MQTT configurations.

5. **Connect to WiFi:**
   Once configured, the ESP8266 will attempt to connect to the specified WiFi network.

6. **Integrate with Home Assistant:**
   The project automatically publishes the relay configurations to the specified MQTT broker, making it easy to integrate with Home Assistant.

## Usage

- Control relays via the web interface or MQTT commands.
- Relay states are published to the MQTT topic for monitoring.
- Use the configured MQTT topics to send commands for turning relays ON or OFF.

## Troubleshooting

- **Connection Issues:** Ensure the correct SSID and password are set and the ESP8266 is within range.
- **MQTT Connectivity:** Verify the MQTT broker settings and ensure it's running.
- **File System Errors:** Check if SPIFFS is mounted properly, and files are accessible.

## License

This project is licensed under the MIT License. Feel free to modify and distribute as needed.

## Author

Developed by [Jkorir]. For contributions and support, please reach out through the repository's issues page.
