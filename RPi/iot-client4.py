from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
import serial
import json
import time

# Setup GPIO for fan control (assumed controlled by Teensy, so no direct GPIO control here)
# Setup Bluetooth serial connection to Teensy (Serial1 on Teensy)
bluetoothSerial = serial.Serial("/dev/rfcomm0", baudrate=9600, timeout=1)

# Setup AWS IoT MQTT client
client = AWSIoTMQTTClient("RaspberryPiClient")
client.configureEndpoint("apkzys4dw1p3u-ats.iot.ap-southeast-2.amazonaws.com", 8883)
client.configureCredentials("/home/maryam/AmazonRootCA1.pem", "/home/maryam/private.pem.key", "/home/maryam/device.pem.crt")

# Set timeouts
client.configureConnectDisconnectTimeout(30)
client.configureMQTTOperationTimeout(20)

# Connect to AWS IoT Core
client.connect()

# Publish received temperature data to AWS IoT Core
def publish_temperature(temperature):
    data = {"temperature": temperature}
    client.publish("device/temperature", json.dumps(data), 1)
    print(f"Published temperature: {temperature}")

# Callback function for when a message is received from the "device/control" topic (Fan ON/OFF)
def on_fan_control_message(client, userdata, message):
    print(f"Received fan control message: {message.payload}")
    fan_status = json.loads(message.payload).get("fan_status")
    
    if fan_status == "ON":
        print(bluetoothSerial.write(b"MANUAL_ON"))  # Send ON command via Bluetooth to Teensy
        print("Sent MANUAL_ON to Teensy")
    elif fan_status == "OFF":
        bluetoothSerial.write(b"MANUAL_OFF")  # Send OFF command via Bluetooth to Teensy
        print("Sent MANUAL_OFF to Teensy")
    else:
        print("Invalid fan status received")

# Subscribe to the fan control topic from AWS IoT Core
client.subscribe("device/control", 1, on_fan_control_message)

while True:
    # Read temperature data from the Teensy via Bluetooth
    temperature_data = bluetoothSerial.readline().decode().strip()

    # Check if we received valid temperature data
    if temperature_data:
        print(f"Received temperature from Teensy: {temperature_data}")
        # Publish the temperature data to AWS IoT Core
        publish_temperature(temperature_data)

    time.sleep(2)  # Delay for 2 seconds before the next reading
