import requests

url = 'http://127.0.0.1:5000/api/data'

# Post request
data = {
    'temperature': 42,
    'humidity': 95
}

# Sending a POST request with JSON data
response = requests.post(url, json=data)

# Debug print. Answer from server
print(response.json())  # response JSON
