import requests

url = 'http://127.0.0.1:5000/api/data'

#url = "https://denoggy.pythonanywhere.com/api/data"

# Post request
data = {
    'temperature': 39,
    'humidity': 10,
    'available': True
}

# Sending a POST request with JSON data
response = requests.post(url, json=data)

# Debug print. Answer from server
print(response.json())  # response JSON
