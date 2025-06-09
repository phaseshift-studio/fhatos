"""
id: mqtt_client
title: MQTT Client
description: Enable an LLM to connect to an MQTT broker in order to publish messages and subscribe to topics.
author: Dr. Stynx
author_url: https://github.com/phaseshift-studio
funding_url: https://github.com/sponsors/phaseshift-studio
version: 0.0.1
license: BSD 3.0
"""
from paho.mqtt import MQTTException
from pydantic import BaseModel, Field
import paho.mqtt.client as mqtt
import json

""" ANSI color codes """
BLACK = '\u001b[0;30m'
RED = '\u001b[0;31m'
GREEN = '\u001b[0;32m'
BROWN = '\u001b[0;33m'
BLUE = '\u001b[0;34m'
PURPLE = '\u001b[0;35m'
CYAN = '\u001b[0;36m'
LIGHT_GRAY = '\u001b[0;37m'
DARK_GRAY = '\u001b[1;30m'
LIGHT_RED = '\u001b[1;31m'
LIGHT_GREEN = '\u001b[1;32m'
MUSTARD = '\u001b[1;33m'
LIGHT_BLUE = '\u001b[1;34m'
LIGHT_PURPLE = '\u001b[1;35m'
# LIGHT_CYAN = "\u001b[1;36m"
# LIGHT_WHITE = "\u001b[1;37m"
BOLD = '\u001b[1m'
# FAINT = "\u001b[2m"
# ITALIC = "\u001b[3m"
# UNDERLINE = "\u001b[4m"
# BLINK = "\u001b[5m"
# NEGATIVE = "\u001b[7m"
# CROSSED = "\u001b[9m"
NC = '\u001b[0m'


class MqttHelper:
    def on_connect(client: mqtt, userdata, flags, reason_code):
        print(f"Connected with result code {reason_code}")
        # Subscribe to the topic when connected
        client.subscribe("#")

    def on_message(client: mqtt, userdata, msg):
        print(f"{msg.topic} {str(msg.payload)}")


class Tools:
    class Valves(BaseModel):
        _VALVE_OVERRIDE_ENVIRONMENT_VARIABLE_NAME_PREFIX = "MQTT_CLIENT_VALVE_OVERRIDE_"
        MQTT_BROKER_URI: str = Field(
            default="localhost",
            description=f"The uri of the mqtt broker; may be overridden by environment variable {_VALVE_OVERRIDE_ENVIRONMENT_VARIABLE_NAME_PREFIX}MQTT_BROKER_URI."
        )
        MQTT_BROKER_PORT: int = Field(
            default=1883,
            description=f"The port of the mqtt broker; may be overridden by environment variable {_VALVE_OVERRIDE_ENVIRONMENT_VARIABLE_NAME_PREFIX}MQTT_BROKER_PORT."
        )
        MQTT_CLIENT: str = Field(
            default="llm_client",
            description=f"The string name of the client connecting to the mqtt broker; may be overridden by environment variable {_VALVE_OVERRIDE_ENVIRONMENT_VARIABLE_NAME_PREFIX}MQTT_CLIENT."
        )
        MQTT_CLIENT_LOG_LEVEL: str = Field(
            default="INFO",
            description=f"The string log level of the mqtt client; may be overridden by environment variable {_VALVE_OVERRIDE_ENVIRONMENT_VARIABLE_NAME_PREFIX}MQTT_CLIENT_LOG_LEVEL."
        )

    def __init__(self):
        self.valves = self.Valves()
        self.citation = True
        self.client = mqtt.Client(
            client_id=self.valves.MQTT_CLIENT, protocol=mqtt.MQTTv311
        )
        self.client.enable_logger()
        self.client.logger.setLevel(self.valves.MQTT_CLIENT_LOG_LEVEL)
        self.client.on_connect = MqttHelper.on_connect
        self.client.on_message = MqttHelper.on_message
        pass

    #########################################################3

    def connect(self, client_id: str | None = None) -> str:
        """
        Connect to an MQTT broker
        """
        if self.client.is_connected():
            return json.dumps(
                {
                    "mqtt_broker": {"uri": self.valves.MQTT_BROKER_URI,
                                    "port": self.valves.MQTT_BROKER_PORT},
                    "mqtt_client": {"id": self.valves.MQTT_CLIENT if client_id is None else client_id,
                                    "log_level": self.valves.MQTT_CLIENT_LOG_LEVEL},
                    "result": "client already connected",
                }
            )
        self.client.logger.info(
            "[{G}MQTT_CLIENT TOOL{NC}] model is connecting to broker %s:%d".format(G=GREEN, NC=NC),
            self.valves.MQTT_BROKER_URI,
            self.valves.MQTT_BROKER_PORT
        )
        if client_id is not None:
            self.client = mqtt.Client(client_id=client_id)
            self.client.enable_logger()
            self.client.logger.setLevel(self.valves.MQTT_CLIENT_LOG_LEVEL)
        result = self.client.connect(self.valves.MQTT_BROKER_URI, self.valves.MQTT_BROKER_PORT)
        self.client.loop_start()
        return json.dumps(
            {
                "mqtt_broker": {"uri": self.valves.MQTT_BROKER_URI,
                                "port": self.valves.MQTT_BROKER_PORT},
                "mqtt_client": {"id": self.valves.MQTT_CLIENT if client_id is None else client_id,
                                "log_level": self.valves.MQTT_CLIENT_LOG_LEVEL},
                "result": mqtt.error_string(result),
            }
        )

    def publish(self, topic: str, message: str, retain: bool = True) -> str:
        """
        Publish a message to a topic on the connected MQTT broker
        """
        if not self.client.is_connected():
            self.connect()
        self.client.logger.info(
            "[{G}MQTT_CLIENT TOOL{NC}] model is publishing %s to the topic %s at %s".format(G=GREEN, NC=NC),
            message,
            topic,
            self.valves.MQTT_BROKER_URI,
        )
        try:
            result = self.client.publish(topic=topic, payload=message, retain=retain)
            result.wait_for_publish(5.0)  # wait for 5 seconds on publish
            return json.dumps(
                {
                    "mqtt_broker": self.valves.MQTT_BROKER_URI,
                    "message": {
                        "message_id": result.mid,
                        "topic": topic,
                        "message": message,
                        "retain": retain,
                        "message_published": result.is_published(),
                    },
                    "result": mqtt.error_string(result.rc)
                }
            )
        except MQTTException as e:
            return json.dumps(
                {
                    "mqtt_broker": self.valves.MQTT_BROKER_URI,
                    "message": {
                        "topic": topic,
                        "message": message,
                        "retain": retain,
                        "message_published": False
                    },
                    "result": str(e)
                }
            )

    def subscribe(self, topic: str) -> str:
        """
        Subscribe to a topic on the connected MQTT broker
        """
        if not self.client.is_connected():
            self.connect()
        self.client.logger.info(
            "[{G}MQTT_CLIENT TOOL{NC}] model is subscribing to the topic %s at %s".format(G=GREEN, NC=NC),
            topic,
            self.valves.MQTT_BROKER_URI
        )
        result = self.client.subscribe(topic)
        return json.dumps(
            {
                "mqtt_broker": self.valves.MQTT_BROKER_URI,
                "topic": topic,
                #    "result": result,
            }
        )
