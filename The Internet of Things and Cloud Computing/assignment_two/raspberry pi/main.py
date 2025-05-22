import time
import serial
import threading
import aws_db

from awscrt import io, mqtt
from awsiot import mqtt_connection_builder

PORT = '/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0'
BAUD_RATE = 9600
DEVICE_ID = "pi_soil_001"
TOPIC_CMD = f"soil/{DEVICE_ID}/read"
ENDPOINT = "a328ros5cm5j9v-ats.iot.ap-southeast-2.amazonaws.com"
CERT_PATH = "pi_soil_001.cert.pem"
KEY_PATH = "pi_soil_001.private.key"
CA_PATH = "root-CA.crt"
CLIENT_ID = "basicPubSub"

ser = None

event_loop = io.EventLoopGroup(1)
resolver = io.DefaultHostResolver(event_loop)
bootstrap = io.ClientBootstrap(event_loop, resolver)

mqtt_conn = mqtt_connection_builder.mtls_from_path(
    endpoint = ENDPOINT,
    cert_filepath = CERT_PATH,
    pri_key_filepath = KEY_PATH,
    ca_filepath = CA_PATH,
    client_bootstrap = bootstrap,
    client_id = CLIENT_ID,
    clean_session = False,
    keep_alive_secs = 30
)


def on_read_command(topic, payload, **kwargs):
    print("MQTT: Called")
    if ser:
        request_immediate(ser)


def request_immediate(ser):
    try:
        ser.write(b'read\n')
        soilmoisture = ser.readline().decode('utf-8', errors = 'replace').strip()
        if soilmoisture:
            aws_db.put_to_dynamo(DEVICE_ID, soilmoisture)
    except Exception as e:
        print("Error:", e)


def serial_read_loop():
    try:
        while True:
            line = ser.readline().decode('utf-8', errors = 'replace').strip()
            if line:
                aws_db.put_to_dynamo(DEVICE_ID, line)
            time.sleep(0.1)
    except Exception as e:
        print("Arduino reading error:", e)


def main():
    global ser

    print("Start...")
    mqtt_conn.connect().result()
    print("MQTT connected")

    subscribe_future, _ = mqtt_conn.subscribe(
        topic = "#",
        qos = mqtt.QoS.AT_LEAST_ONCE,
        callback = on_read_command)
    subscribe_future.result()
    print(f"Subscribe to: {TOPIC_CMD}")

    ser = serial.Serial(PORT, BAUD_RATE, timeout = 2)
    time.sleep(2)

    serial_thread = threading.Thread(target = serial_read_loop, daemon = True)
    serial_thread.start()

    try:
        print("Ready")
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Finish...")
    finally:
        if ser:
            ser.close()
        mqtt_conn.disconnect().result()
        print("Disconnected MQTT")


if __name__ == "__main__":
    main()

