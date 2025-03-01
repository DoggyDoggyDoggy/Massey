from time import sleep
import board
import adafruit_dht
import requests

dhtDevice = adafruit_dht.DHT11(board.D17)
url = "https://denoggy.pythonanywhere.com/api/data"

api_link = "https://api.telegram.org/bot"
api_key = "7671876653:AAGMvr1iSa7KhNNIVloBKMl8bSweLs5Zx_M"
chat_id = "1079159406"

telegram_url = f"{api_link}{api_key}/sendMessage?chat_id={chat_id}&text="

data = {
    'available': True,
    'temperature': None,
    'humidity': None
}


def send_warning_message(text):
    requests.get((telegram_url + text))


def send_data():
    # Sending a POST request with JSON data
    response = requests.post(url, json = data)

    # Debug print. Answer from server
    print(response.json())  # response JSON


while True:

    try:
        data['temperature'] = dhtDevice.temperature
        data['humidity'] = dhtDevice.humidity
    except RuntimeError as error:
        print("RuntimeError:", error)
        continue
    except Exception as error:
        dhtDevice.exit()
        data['available'] = False,
        send_warning_message(text = "Exception%20error!")
        send_data()
        raise error

    send_data()

    if data['temperature'] > 40 and data['humidity'] > 90:
        send_warning_message(
            text = "Warning message!%0AHumidity%20is%20over%2090%%20and%20temperature%20is%20over%2040°C"
        )
    elif data['temperature'] > 40:
        send_warning_message(
            text = "Warning message!%0ATemperature%20is%20over%2040°C"
        )
    elif data['humidity'] > 90:
        send_warning_message(
            text = "Warning message!%0AHumidity%20is%20over%2090%"
        )

    sleep(60)
