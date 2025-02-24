from time import sleep
import board
import adafruit_dht
import requests

dhtDevice = adafruit_dht.DHT11(board.D17)
url = "https://denoggy.pythonanywhere.com/api/data"

data = {
    'available': True,
    'temperature': None,
    'humidity': None
}


def send_data():
    # Sending a POST request with JSON data
    response = requests.post(url, json = data)

    # Debug print. Answer from server
    print(response.json())  # response JSON


while True:

    try:
        data['temperature'] = dhtDevice.temperature,
        data['humidity'] = dhtDevice.humidity
    except RuntimeError as error:
        print("RuntimeError:", error)
        continue
    except Exception as error:
        dhtDevice.exit()
        data['available'] = False,
        send_data()
        raise error

    send_data()

    sleep(60)
