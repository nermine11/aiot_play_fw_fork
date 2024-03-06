import json
import traceback
import paho.mqtt.client as mqtt

MQTTTOPIC = "aiotsystems"

def mqtt_on_connect(client, userdata, flags, rc):
    client.subscribe(MQTTTOPIC)
    print("MQTT connected")

def mqtt_on_message(client, userdata, msg):
    try:
        mqttmsg = json.loads(msg.payload.decode('ascii'))
        print(f'from MQTT: {mqttmsg}')
    except:
        print(f'ERROR mqtt_on_message: {traceback.format_exc()}')

mqtt_client            = mqtt.Client()
mqtt_client.on_connect = mqtt_on_connect
mqtt_client.on_message = mqtt_on_message
mqtt_client.connect("broker.mqttdashboard.com", 1883, 60)
mqtt_client.loop_start()

input('Press Enter to close.')
