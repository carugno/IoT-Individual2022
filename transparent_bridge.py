from ast import parse
from operator import le
import paho.mqtt.client as mqtt
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
import json

#AWS Paramater Init
AWS_HOST = "PUT HERE YOUR AMAZON ENDPOINT" 
AWS_ROOT_CAP_PATH = "PUT HERE THE ABSOLUTE PATH TO root-CA.crt "
AWS_CERTIFICATE_PATH = "PUT HERE THE ABSOLUTE PATH TO nucleo-board.cert.pem"
AWS_PRIVATE_KEY_PATH = "PUT HERE THE ABSOLUTE PATH TO nucleo-board.private.key"
AWS_PORT = 8883
AWS_CLIENT_ID = "nucleo"


#MQTT Broker Parameters
MQTT_SERVER = "localhost"
MQTT_PORT = 1886
MQTT_KEEP_ALIVE = 60


TOPIC_FROM_BOARD = "out"
TOPIC_TEMP_TO_AWS = "temperature"

def mqtt_publish(mqtt_client, topic, message):
    mqtt_client.publish(topic, message)

# Function called to send messages to AWS, invoked when a msg is received from the MQTT Broker
def aws_publish(aws_client, message):
    message_out = {}
    message = message.split("h")
    message_out['temperature'] =  message[0][1:len(message[0])-1]+"."+ message[0][len(message[0])-1:]
    message_out['humidity'] = message[1][:len(message[1])-1]+"."+ message[1][len(message[1])-1:]
    messageJson = json.dumps(message_out)
    aws_client.publish(TOPIC_TEMP_TO_AWS, messageJson, 1)
    
print("Transparent Bridge Started\n")
#AWS and MQTT Objects
AWS_CLIENT = None
MQTT_CLIENT = None
#AWS Start Init
AWS_CLIENT = AWSIoTMQTTClient(AWS_CLIENT_ID)
AWS_CLIENT.configureEndpoint(AWS_HOST, AWS_PORT)
AWS_CLIENT.configureCredentials(AWS_ROOT_CAP_PATH, AWS_PRIVATE_KEY_PATH, AWS_CERTIFICATE_PATH)

AWS_CLIENT.connect()
#MQTT Start Init 
MQTT_CLIENT = mqtt.Client()
MQTT_CLIENT.connect(MQTT_SERVER, MQTT_PORT, MQTT_KEEP_ALIVE) 
MQTT_CLIENT.loop_forever()
