# import context  # Ensures paho is in PYTHONPATH
import paho.mqtt.client as mqtt
from pydantic import BaseModel, Field


def on_connect(mqttc, obj, flags, reason_code, properties):
    print("reason_code: " + str(reason_code))


def on_message(mqttc, obj, msg):
    print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))


def on_subscribe(mqttc, obj, mid, reason_code_list, properties):
    print("Subscribed: " + str(mid) + " " + str(reason_code_list))


def on_log(mqttc, obj, level, string):
    print(string)


class Tools:
    """
    tool = Tools()
    tool.connect("fhat.local")
    tool.subscribe("#")
    tool.loop()
    """

    def __init__(self):
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        pass

    def connect(self, broker: str, port: int = 1883) -> bool:
        """
        connect to a mqtt broker
        """
        self.client.on_connect = on_connect
        self.client.on_message = on_message
        self.client.on_log = on_log
        self.client.on_subscribe = on_subscribe
        error_code = self.client.connect(broker, port)
        if error_code == 0:
            print("connection to", broker, "successful")
        return 0 == error_code

    def subscribe(self, topic: str = Field("#", description="the topic to subscribe to")) -> bool:
        """
        subscribe to a topic
        """
        self.client.subscribe(topic)
        return True

    def loop(self):
        self.client.loop_forever()

tool = Tools()
tool.connect("fhat.local")
tool.subscribe("#")
tool.loop()