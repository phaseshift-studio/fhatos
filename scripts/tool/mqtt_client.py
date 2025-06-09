"""
id: mqtt_client
title: MQTT OpenWeb UI LLM Client Tool
description: Enable an LLM to connect to an MQTT broker in order to publish messages and subscribe to topics.
author: Dr. Stynx
author_url: https://github.com/phaseshift-studio
funding_url: https://github.com/sponsors/phaseshift-studio
version: 0.0.1
license: BSD 3.0
requirements: paho-mqtt
"""
from typing import Any, Callable, Optional
from pydantic import BaseModel, Field
import paho.mqtt.client as mqtt
from paho.mqtt import MQTTException
import json

##############################
###### ANSI Color Codes ######
##############################
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
BOLD = '\u001b[1m'
NC = '\u001b[0m'


##############################
##############################


# (EventEmitter class definition as provided in the template notes)
class EventEmitter:
    def __init__(self, event_emitter: Callable[[dict], Any] = None):
        self.event_emitter = event_emitter

    async def progress_update(self, description: str):
        await self.emit_status("status", description)

    async def error_update(self, description: str):
        await self.emit_status("status", description, "error", True)

    async def success_update(self, description: str):
        await self.emit_status("status", description, "success", True)

    async def notify(self, type: str = "info", content: str = ""):
        await self.emit_notification(type, content)

    async def emit_notification(
            self,
            type: str = "info",
            content: str = "no content",
    ):
        if self.event_emitter:
            await self.event_emitter(
                {
                    "type": "notification",
                    "data": {
                        "type": type,
                        "content": content,
                    },
                }
            )

    async def emit_status(
            self,
            description: str = "no description",
            status: str = "in_progress",
            done: bool = False,
    ):
        if self.event_emitter:
            await self.event_emitter(
                {
                    "type": "status",
                    "data": {
                        "status": status,
                        "description": description,
                        "done": done,
                    },
                }
            )

    async def append_message(
            self,
            content: str = "no content",
    ):
        if self.event_emitter:
            await self.event_emitter(
                {
                    "type": "chat:message:delta",
                    "data": {
                        "content": content
                    },
                }
            )


class MqttHelper:
    def __init__(self, event_emitter: EventEmitter):
        self.event_emitter = event_emitter

    def on_connect(self, client: mqtt, userdata, flags, reason_code):
        if 0 == reason_code:
            self.event_emitter.notify(type="info",
                                      content="model connected to broker")
        client.subscribe("#")

    def on_message(self, client: mqtt, userdata, msg):
        print(f"{msg.topic} {str(msg.payload)}")
        self.event_emitter.emit_notification(type="info", content=f"{msg.topic} {str(msg.payload)}")


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
        OLLAMA_SERVER_URI: str = Field(
            default="localhost",
            description=f"The uri of the ollama server REST endpoint; may be overridden by environment variable {_VALVE_OVERRIDE_ENVIRONMENT_VARIABLE_NAME_PREFIX}OLLAMA_SERVER_URI."
        )
        OLLAMA_SERVER_PORT: int = Field(
            default=11434,
            description=f"The port of the ollama server REST endpoint; may be overridden by environment variable {_VALVE_OVERRIDE_ENVIRONMENT_VARIABLE_NAME_PREFIX}OLLAMA_PORT_URI."
        )

    def __init__(self):
        self.valves = self.Valves()
        self.citation = True
        self.helper = None
        self.client = mqtt.Client(
            client_id=self.valves.MQTT_CLIENT, protocol=mqtt.MQTTv311
        )
        self.client.enable_logger()
        self.client.logger.setLevel(self.valves.MQTT_CLIENT_LOG_LEVEL)
        pass

    #########################################################3

    async def connect(self, client_id: Optional[str] = None,
                      __event_emitter__: Callable[[dict], Any] = None) -> str:
        """
        Connect to an MQTT broker
        """
        event_emitter = EventEmitter(__event_emitter__)
        self.helper = MqttHelper(event_emitter)
        self.client.on_connect = self.helper.on_connect
        self.client.on_message = self.helper.on_message
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
        await event_emitter.notify(type="success",
                                   content="model is connected to broker {broker}:{port}".format(
                                       broker=self.valves.MQTT_BROKER_URI,
                                       port=self.valves.MQTT_BROKER_PORT))
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

    async def publish(self, topic: str, message: str, retain: bool = True,
                      __event_emitter__: Callable[[dict], Any] = None) -> str:
        """
        Publish a message to a topic on the connected MQTT broker
        """
        if not self.client.is_connected():
            await self.connect(client_id=None, __event_emitter__=__event_emitter__)
        self.client.logger.info(
            "[{G}MQTT_CLIENT TOOL{NC}] model is publishing %s to the topic %s at %s".format(G=GREEN, NC=NC),
            message,
            topic,
            self.valves.MQTT_BROKER_URI,
        )
        try:
            result = self.client.publish(topic=topic, payload=message, retain=retain)
            event_emitter = EventEmitter(__event_emitter__)
            await event_emitter.notify(type="info", content="model published to {topic}: {payload}".format(topic=topic,
                                                                                                           payload=message))
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

    def subscribe(self, topic: str, __event_emitter__: Callable[[dict], Any] = None) -> str:
        """
        Subscribe to a topic on the connected MQTT broker
        """
        if not self.client.is_connected():
            self.connect(client_id=None, __event_emitter__=__event_emitter__)
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
