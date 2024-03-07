import json
import traceback
import paho.mqtt.client as mqtt
import struct

MQTTTOPIC = "aiotsystems"

def parse_sht31(message):
    
    data        = message['payload']['fields']['data']       # shorthand
    macAddress  = message['payload']['fields']['macAddress'] # shorthand
    moteLabel   = '-'.join(macAddress.split('-')[-3:])

    assert(len(data)==4)
    
    # parse
    (temperature_raw,humidity_raw) = struct.unpack('<HH', bytes(data))

    # convert
    divisor     = 2**16 - 1
    temperature = -45 + 175. * temperature_raw / divisor
    humidity    = 100. * humidity_raw / divisor

    return (moteLabel, temperature, humidity)

def mqtt_on_connect(client, userdata, flags, rc):
    client.subscribe(MQTTTOPIC)
    print("MQTT connected")

def mqtt_on_message(client, userdata, msg):
    try:
        # raw message
        mqttmsg = json.loads(msg.payload.decode('ascii'))
        print(f'from MQTT: {mqttmsg}')
        
        # parse SHT31 data
        if mqttmsg['payload']['name']=='notifData':
            (moteLabel, temperature, humidity) = parse_sht31(mqttmsg)
            print(f'mote {moteLabel} reports temperature={temperature:>7.03f} C humidity={humidity:>7.03f} %')
        
    except:
        print(f'ERROR mqtt_on_message: {traceback.format_exc()}')

mqtt_client            = mqtt.Client()
mqtt_client.on_connect = mqtt_on_connect
mqtt_client.on_message = mqtt_on_message
mqtt_client.connect("broker.mqttdashboard.com", 1883, 60)
mqtt_client.loop_start()

input('Press Enter to close.')
