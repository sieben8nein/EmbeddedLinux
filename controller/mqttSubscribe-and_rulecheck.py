from paho.mqtt import client as mqtt_client

broker = 'localhost'
port = 1883
tempTopic = "temp"
humidityTopic = "humidity"
co2Topic = "co2"
moistureTopic = "moisture"
client_id = 'python-mqtt-rulechecker'
username = 'my_user'
password = 'bendevictor'
manual = 0
ESP8266Active = False
ESP32Active = False
scheduledMessages = []
def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client, topic):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        ruleCheck(msg.payload.decode(), msg.topic, client)

    client.subscribe(topic)
    client.on_message = on_message

def publish(client,topic, message):
    msg = message
    if manual == 0:
        result = client.publish(topic, msg)
        # result: [0, 1]
        status = result[0]
        if status == 0:
            print(f"Send `{msg}` to topic `{topic}`")
        else:
            print(f"Failed to send message to topic {topic}")
    

def ruleCheck(value, topic, client):
    if topic == "manual":
        global manual 
        manual = int(value)

    if topic == 'ESP8266':
        global ESP8266Active 
        ESP8266Active = bool(value)
    if topic == 'ESP32':
        global ESP32Active 
        ESP32Active = bool(value)
    if topic == "temp":
        if (float(value) > 25) and (float(value) <= 30):
            scheduledMessages.append(["ESP8266Active", "tempActuator", 175])
        elif ((float(value) > 30) and (float(value) <= 35)):
            scheduledMessages.append(["ESP8266Active", "tempActuator", 200])
        elif float(value) > 35:
            scheduledMessages.append(["ESP8266Active", "tempActuator", 255])
        else:
            scheduledMessages.append(["ESP8266Active", "tempActuator", 0])
        
    elif topic == "humidity":
        if float(value) > 30:
            scheduledMessages.append(["ESP8266Active", "dehumidifierActuator", "open"])
        else:
            scheduledMessages.append(["ESP8266Active", "dehumidifierActuator", "close"])

    elif topic == "co2":
        if float(value) > 1000:
            scheduledMessages.append(["ESP8266Active", "windowActuator", "open"])
        else:
            scheduledMessages.append(["ESP8266Active", "windowActuator", "close"])

    elif topic == "moisture":
        if float(value) < 850:
            publish(client, "pumpActuator", 255)
        elif float(value) > 850:
            publish(client, "pumpActuator", 0)
    return
def HandleScheduled(client):
    global scheduledMessages
    for msg in scheduledMessages:
        if msg[0] == "ESP8266Active":
            if ESP8266Active:
                publish(client, msg[1], msg[2])
                scheduledMessages.remove(msg)
        if msg[0] == "ESP32Active":
            if ESP32Active:
                publish(client, msg[1], msg[2])
                scheduledMessages.remove(msg)

def run():
    client = connect_mqtt()
    subscribe(client, tempTopic)
    subscribe(client, humidityTopic)
    subscribe(client, co2Topic)
    subscribe(client, moistureTopic)
    subscribe(client, "manual")
    client.loop_forever()
    


if __name__ == '__main__':
    run()