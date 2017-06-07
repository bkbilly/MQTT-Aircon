"""
Support for MQTT heaters.

For more details about this platform, please refer to the documentation at
https://home-assistant.io/components/switch.mqtt/
"""
import logging
import voluptuous as vol
from homeassistant.components.mqtt import (CONF_QOS, CONF_RETAIN)
from homeassistant.components.climate import (
    ClimateDevice, ATTR_OPERATION_MODE, ATTR_TARGET_TEMP_HIGH,
    ATTR_TARGET_TEMP_LOW, ATTR_OPERATION_LIST, ATTR_FAN_MODE,
    ATTR_FAN_LIST, ATTR_SWING_MODE, ATTR_SWING_LIST)

from homeassistant.const import (
    CONF_NAME, TEMP_CELSIUS, ATTR_TEMPERATURE)
import homeassistant.components.mqtt as mqtt
import homeassistant.helpers.config_validation as cv

_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ['mqtt']

DEFAULT_NAME = 'MQTT Airconditioner'

PLATFORM_SCHEMA = mqtt.MQTT_RW_PLATFORM_SCHEMA.extend({
    vol.Optional(CONF_NAME, default=DEFAULT_NAME): cv.string,
})


def setup_platform(hass, config, add_devices, discovery_info=None):
    """Setup the MQTT climate device."""
    add_devices([MqttClimate(
        hass,
        config.get(CONF_NAME),
        config.get('min_temp'),
        config.get('max_temp'),
        config.get('command_topic'),
        config.get('measured_temp_topic'),
        config.get('command_temp_topic'),
        config.get('command_mode_topic'),
        config.get('command_speed_topic'),
        config.get('command_swing_topic'),
        config.get('command_sleep_topic'),
        config.get('current_temp_topic'),
        config.get('current_mode_topic'),
        config.get('current_speed_topic'),
        config.get('current_swing_topic'),
        config.get('current_sleep_topic'),
    )])


class MqttClimate(ClimateDevice):
    """Representation of a heater that can be toggled using MQTT."""

    def __init__(self, hass, name,
                 min_temp, max_temp,
                 command_topic,
                 measured_temp_topic,
                 command_temp_topic,
                 command_mode_topic,
                 command_speed_topic,
                 command_swing_topic,
                 command_sleep_topic,
                 current_temp_topic,
                 current_mode_topic,
                 current_speed_topic,
                 current_swing_topic,
                 current_sleep_topic):
        """Initialize the MQTT switch."""
        self._state = False
        self._hass = hass
        self._name = name

        self._operation_list = ["Off", "Heat", "Cool", "Dry", "Fan", "Auto"]
        self._fan_list = ["0", "1", "2", "3", "4"]
        self._swing_list = ["Off", "On"]

        self._min_temp = min_temp
        self._max_temp = max_temp

        self._command_topic = command_topic
        self._command_mode_topic = command_mode_topic
        self._command_speed_topic = command_speed_topic
        self._command_swing_topic = command_swing_topic
        self._command_sleep_topic = command_sleep_topic
        self._command_temp_topic = command_temp_topic

        self._measured_temp_topic = measured_temp_topic

        self._current_temp_topic = current_temp_topic
        self._current_mode_topic = current_mode_topic
        self._current_speed_topic = current_speed_topic
        self._current_swing_topic = current_swing_topic
        self._current_sleep_topic = current_sleep_topic

        self._measured_temp = None
        self._current_temp = None
        self._current_mode = None
        self._current_speed = None
        self._current_swing = None
        self._current_sleep = None

        def message_received(topic, payload, qos):
            """A new MQTT message has been received."""
            if topic == self._measured_temp_topic:
                self._measured_temp = payload
            elif topic == self._current_temp_topic:
                self._current_temp = int(payload)
            elif topic == self._current_mode_topic:
                self._current_mode = payload
            elif topic == self._current_speed_topic:
                self._current_speed = payload
            elif topic == self._current_swing_topic:
                self._current_swing = payload
            elif topic == self._current_sleep_topic:
                print(payload)
                self._current_sleep = payload
            else:
                print("unknown topic")
            self.schedule_update_ha_state()

        subscribeTopics = [
            self._measured_temp_topic,
            self._current_temp_topic,
            self._current_mode_topic,
            self._current_speed_topic,
            self._current_swing_topic,
            self._current_sleep_topic,
        ]
        for topic in subscribeTopics:
            mqtt.subscribe(
                hass, topic, message_received, 0)

        mqtt.async_publish(self._hass, self._command_topic,
                           "update", 0, True)

    @property
    def should_poll(self):
        """Polling not needed for a demo climate device."""
        return False

    @property
    def name(self):
        """Return the name of the climate device."""
        return self._name

    @property
    def temperature_unit(self):
        """Return the unit of measurement."""
        return TEMP_CELSIUS

    @property
    def min_temp(self):
        """Return the minimum temperature."""
        # pylint: disable=no-member
        if self._min_temp:
            return self._min_temp
        else:
            # get default temp from super class
            return ClimateDevice.min_temp.fget(self)

    @property
    def max_temp(self):
        """Return the maximum temperature."""
        # pylint: disable=no-member
        if self._max_temp:
            return self._max_temp
        else:
            # Get default temp from super class
            return ClimateDevice.max_temp.fget(self)

    @property
    def target_temperature_step(self):
        """Return the supported step of target temperature."""
        return 1

    # ******** Mode ********
    @property
    def current_operation(self):
        """Return current operation ie. heat, cool, idle."""
        return self._current_mode

    def set_operation_mode(self, operation_mode):
        """Set new operation mode."""
        self._current_mode = operation_mode
        self._publish_mode()
        self.schedule_update_ha_state()

    def _publish_mode(self):
        if self._current_mode is None:
            return
        mqtt.publish(self._hass, self._command_mode_topic,
                     self._current_mode, 0, True)

    @property
    def operation_list(self):
        """List of available operation modes."""
        return self._operation_list

    ******** Temp ********
    @property
    def current_temperature(self):
        """Return the temperature we try to reach."""
        return self._measured_temp

    @property
    def target_temperature(self):
        """Return the temperature we try to reach."""
        return self._current_temp

    def set_temperature(self, **kwargs):
        """Set new target temperatures."""
        if kwargs.get(ATTR_TEMPERATURE) is not None:
            self._current_temp = int(kwargs.get(ATTR_TEMPERATURE))
        self._publish_temp()
        self.schedule_update_ha_state()

    def _publish_temp(self):
        if self._current_temp is None:
            return
        mqtt.publish(self._hass, self._command_temp_topic,
                     self._current_temp, 0, True)

    # ******** Speed ********
    @property
    def current_fan_mode(self):
        """Return the fan setting."""
        return self._current_speed

    @property
    def fan_list(self):
        """List of available fan modes."""
        return self._fan_list

    def set_fan_mode(self, fan_mode):
        """Set new fan mode."""
        self._current_speed = fan_mode
        self._publish_speed()
        self.schedule_update_ha_state()

    def _publish_speed(self):
        if self._current_speed is None:
            return
        mqtt.publish(self._hass, self._command_speed_topic,
                     self._current_speed, 0, True)

    # ******** Swing ********
    @property
    def current_swing_mode(self):
        """Return the swing setting."""
        return self._current_swing

    @property
    def swing_list(self):
        """List of available swing modes."""
        return self._swing_list

    def set_swing_mode(self, swing_mode):
        """Set new swing mode."""
        self._current_swing = swing_mode
        self._publish_swing()
        self.schedule_update_ha_state()

    def _publish_swing(self):
        if self._current_swing is None:
            return
        mqtt.publish(self._hass, self._command_swing_topic,
                     self._current_swing, 0, True)


    # Doesn't recognize for some reason the off state...
    # # ******** Sleep (Away Mode) ********
    # @property
    # def is_away_mode_on(self):
    #     """Return if away mode is on."""
    #     return self._current_sleep

    # def turn_away_mode_on(self):
    #     """Turn away mode on."""
    #     print("Changing to ON")
    #     self._current_sleep = False
    #     self._publish_sleep()
    #     self.schedule_update_ha_state()

    # def turn_away_mode_off(self):
    #     """Turn away mode off."""
    #     print("Changing to OFF")
    #     self._current_sleep = True
    #     self._publish_sleep()
    #     self.schedule_update_ha_state()

    # def _publish_sleep(self):
    #     print("sending the data: ", self._current_sleep)
    #     if self._current_sleep is None:
    #         return
    #     mqtt.publish(self._hass, self._command_sleep_topic,
    #                  self._current_sleep, 0, True)

    # # ******** Sleep (Heat Mode) ********
    # @property
    # def is_aux_heat_on(self):
    #     """Return true if aux heater."""
    #     return self._current_sleep

    # def turn_aux_heat_on(self):
    #     """Turn auxillary heater on."""
    #     print("Changing to ON")
    #     self._current_sleep = False
    #     self._publish_sleep()
    #     self.schedule_update_ha_state()

    # def turn_aux_heat_off(self):
    #     """Turn auxillary heater off."""
    #     print("Changing to OFF")
    #     self._current_sleep = True
    #     self._publish_sleep()
    #     self.schedule_update_ha_state()

