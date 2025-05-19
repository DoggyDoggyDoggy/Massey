import serial
import time
import aws_db

PORT = '/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0'
BAUD_RATE = 9600
device_id = "pi-soil-001"

def main():
    ser = serial.Serial(PORT, BAUD_RATE, timeout = 1)
    time.sleep(2)
    try:
        while True:
            soilmoisture = ser.readline().decode('utf-8', errors = 'replace').strip()
            if soilmoisture:
                aws_db.put_to_dynamo(device_id, soilmoisture)
            break
    except KeyboardInterrupt:
        pass
    finally:
        ser.close()


if __name__ == '__main__':
    main()