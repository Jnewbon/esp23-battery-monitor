| Supported Targets | ESP32-C6 | ESP32-H2 | 
| ----------------- | -------- | -------- | 

# Zigbee 12v Battery monitor

This is designed to work in the zigbee mesh network to report the voltage of a 12v battery, intended for use in vehicles. it will focus of low power consumption while reporting the level

This is intended to be used wit a 5/1 voltage devider to reduce 12v down to the measurable range for the ESP32 ADC pin

## How to use example

Before project configuration and build, be sure to set the correct chip target using `idf.py set-target <chip_name>`.

### Hardware Required

* A development board with zigbee enabled SoC (eg ESP32-C6)
* A USB cable for power supply and programming

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

```
TBC

```

## Troubleshooting

TBC
