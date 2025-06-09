"""
id: mqtt_client
title: MQTT Client
description: Enable an LLM to connect to an MQTT broker in order to publish messages and subscribe to topics.
author: Dr. Stynx
author_url: http:://markorodriguez.com
funding_url: https://github.com/okram
version: 0.0.1
license: BSD 3.0
"""

from pydantic import BaseModel, Field
import paho.mqtt.client as mqtt
import json


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
            description=f"The uri of the mqtt broker; may be overridden by environment variable {_VALVE_OVERRIDE_ENVIRONMENT_VARIABLE_NAME_PREFIX}MQTT_BROKER_URI.",
        )
        MQTT_BROKER_PORT: int = Field(
            default=1883,
            description=f"The port of the mqtt broker; may be overridden by environment variable {_VALVE_OVERRIDE_ENVIRONMENT_VARIABLE_NAME_PREFIX}MQTT_BROKER_PORT.",
        )
        MQTT_CLIENT: str = Field(
            default="llm_client",
            description=f"The string name of the client connecting to the mqtt broker; may be overridden by environment variable {_VALVE_OVERRIDE_ENVIRONMENT_VARIABLE_NAME_PREFIX}MQTT_CLIENT.",
        )

    def __init__(self):
        self.connected = False
        self.valves = self.Valves()
        self.citation = True
        self.client = mqtt.Client(
            client_id=self.valves.MQTT_CLIENT, protocol=mqtt.MQTTv311
        )
        self.client.on_connect = MqttHelper.on_connect
        self.client.on_message = MqttHelper.on_message
        pass

    #########################################################3

    def connect(self) -> str:
        """
        Connect to an MQTT broker
        """
        if self.connected:
            return "mqtt client already connected"
        print(
            "[MQTT_CLIENT TOOL] model is connecting to broker:",
            self.valves.MQTT_BROKER_URI,
        )
        self.client.connect(
            self.valves.MQTT_BROKER_URI, self.valves.MQTT_BROKER_PORT, 60
        )
        self.connected = True
        self.client.loop_start()
        return "mqtt client connected"

    def publish(self, topic: str, message: str, retain: bool = True) -> str:
        """
        Publish a message to a topic on the connected MQTT broker
        """
        if not self.connected:
            self.connect()
        print(
            "[MQTT_CLIENT TOOL] model is publishing",
            message,
            "to the topic",
            topic,
            "at",
            self.valves.MQTT_BROKER_URI,
        )
        result = self.client.publish(topic=topic, payload=message, retain=retain)
        return json.dumps(
            {
                "mqtt_broker": self.valves.MQTT_BROKER_URI,
                "topic": topic,
                "message": message,
                "retain": retain,
                #  "result": result,
            }
        )

    def subscribe(self, topic: str) -> str:
        """
        Subscribe to a topic on the connected MQTT broker
        """
        if not self.connected:
            self.connect()
        print(
            "[MQTT_CLIENT TOOL] model is subscribing to the topic",
            topic,
            "at",
            self.valves.MQTT_BROKER_URI,
        )
        result = self.client.subscribe(topic)
        return json.dumps(
            {
                "mqtt_broker": self.valves.MQTT_BROKER_URI,
                "topic": topic,
                #    "result": result,
            }
        )
