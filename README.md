# iot_node

This repository is part of my undergraduate monograph. The objectives of the work were:

Data collection using IoT nodes
Analyze the consumption of electricity in a home and correlate them with environmental variables
The data collection was made using IoT nodes based on ESP32, and a Python [MQTT client](https://github.com/XxKavosxX/mqtt_client) to persist data in a AWS database.

The data collected was cleaned and analysed using python scripts and jupyter notebooks presented in [this](https://github.com/XxKavosxX/consumption_data_analysis) repository.


This is a IOT node using esp32-wroom as microncotroller and MQTT protocol for data exchange.

# Sensors libraries available in *src/*:
- ZMPT101b
- SCT013
- DHT11
- SHTC3

