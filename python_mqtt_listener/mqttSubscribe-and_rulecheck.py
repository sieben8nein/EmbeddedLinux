from paho.mqtt import client as mqtt_client

broker = 'localhost'
port = 1883
topic1 = "temp"
topic2 = "humidity"
topic3 = "co2"
client_id = 'python-mqtt-rulechecker'
username = 'my_user'
password = 'bendevictor'

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
    result = client.publish(topic, msg)
    # result: [0, 1]
    status = result[0]
    if status == 0:
        print(f"Send `{msg}` to topic `{topic}`")
    else:
        print(f"Failed to send message to topic {topic}")
    

def ruleCheck(value, topic, client):
    if topic == "temp":
        if float(value) > 25:
            publish(client, "tempActuator", "open")
        else:
            publish(client, "tempActuator", "close")
        
    elif topic == "humidity":
        if float(value) > 30:
            publish(client, "dehumidifierActuator", "open")
        else:
            publish(client, "dehumidifierActuator", "close")
    elif topic == "co2":
        if float(value) > 1200:
            publish(client, "windowActuator", "open")
        else:
            publish(client, "windowActuator", "close")
    return

def run():
    client = connect_mqtt()
    subscribe(client, topic1)
    subscribe(client, topic2)
    subscribe(client, topic3)
    client.loop_forever()
    


if __name__ == '__main__':
    run()