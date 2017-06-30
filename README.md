# MQTT Airconditioner

Control the Airconditioner from HomeAssistant with an ESP8266 using an IR Led.
These commands work on my Carrefour Airconditioner.

## Dependencies Arduino
 * [IRremoteESP8266](https://github.com/markszabo/IRremoteESP8266)
 * [PubSubClient](http://pubsubclient.knolleary.net/)
 * [DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
 * [Adafruit Unified Sensor libraries](https://github.com/adafruit/Adafruit_Sensor)

## Install Home Assistant Component

 * copy mqtt.py file to this location: ~/.homeassistant/custom_components/climate/
 * Edit your configuration located ~/.homeassistant/configuration.yaml
```yaml
climate:
  - platform: mqtt
    name: "mqtt-aircon"
    min_temp: 16
    max_temp: 30
    command_topic: "/aircond/myroom/getall"
    measured_temp_topic: "/aircond/myroom/temp/measured"
    command_temp_topic: "/aircond/myroom/temp/set"
    command_mode_topic: "/aircond/myroom/mode/set"
    command_speed_topic: "/aircond/myroom/speed/set"
    command_swing_topic: "/aircond/myroom/swing/set"
    command_sleep_topic: "/aircond/myroom/sleep/set"
    current_temp_topic: "/aircond/myroom/temp"
    current_mode_topic: "/aircond/myroom/mode"
    current_speed_topic: "/aircond/myroom/speed"
    current_swing_topic: "/aircond/myroom/swing"
    current_sleep_topic: "/aircond/myroom/sleep"
```
