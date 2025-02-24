from time import sleep
import board
import adafruit_dht
import requests

dhtDevice = adafruit_dht.DHT11(board.D17)
url = "https://denoggy.pythonanywhere.com/api/data"

while True:
    data = {
        'temperature': None,
        'humidity': None
    }
    try:
        data['temperature'] = dhtDevice.temperature,
        data['humidity'] = dhtDevice.humidity
    except RuntimeError as error:
        print("RuntimeError:", error)
        continue
    except Exception as error:
        dhtDevice.exit()
        raise error

    # Sending a POST request with JSON data
    response = requests.post(url, json = data)

    # Debug print. Answer from server
    print(response.json())  # response JSON

    sleep(60)