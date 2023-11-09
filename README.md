
# lightbox
https://github.com/drfhaust/lightbox

This project is designed to create an intelligent power management system capable of handling multiple power sources, including mains electricity (NEPA), an inverter, and a generator. It aims to automate the process of switching between these power sources based on availability and predefined criteria, as well as to provide remote monitoring and control capabilities.

### Key Components and Libraries Used

- **Microcontroller**: ESP8266 for WiFi capabilities and general control logic.
- **WiFi Communication**: ESP-NOW protocol for efficient, low-latency communication between ESP8266 modules.
- **GSM Module**: For cellular network communication, enabling remote monitoring and control via MQTT.
- **OLED Display**: SSD1306 OLED display for real-time status updates.
- **Power Sensors**: To measure voltage and current from different power sources.
- **Relays and Servos**: For physically switching between power sources.
- **Energy Monitoring**: Using the EmonLib library to measure electrical parameters such as voltage, current, and power.
- **Persistent Storage**: EEPROM to store the last state and other persistent data.
- **Other Libraries**: ArduinoJson for JSON handling, TinyGsmClient for GSM functionality, PubSubClient for MQTT communication, and Adafruit GFX libraries for the display.

### Parameters Measured

- **Voltage**: Measuring the ac voltage level of each power source to determine availability and stability.
- **Current**: Monitoring the current draw for load calculations and detecting potential overloads.
- **Power**: Calculating power consumption for energy monitoring and management.

### Controlled Inputs

- **Power Source Selection**: Automatic or manual selection of the power source based on user input or remote commands.
- **Generator Control**: Start and stop the generator automatically depending on the power requirements and status of other power sources.
- **Load Management**: Disconnecting or reconnecting loads based on current draw and power source status to prevent overloads.

### Operation Modes

- **Auto Mode**: The system automatically switches between NEPA, inverter, and generator based on predefined logic and sensor inputs.
- **Manual Mode**: The user can manually override the automatic system to select a preferred power source.
- **Remote Control**: Through GSM and MQTT, the system can receive commands to change modes or perform specific actions like starting the generator.

### Safety and Fault Tolerance

The system includes logic to detect faults in each power source, such as under-voltage, over-voltage, or overload conditions. It can switch off a faulty power source to protect the loads and switch to an alternate power source if available.

---
