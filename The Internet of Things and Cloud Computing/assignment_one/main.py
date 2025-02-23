from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/api/data', methods=['POST'])
def receive_data():
    if request.is_json:
        data = request.get_json()  # Get data JSON
        temperature = data.get('temperature')
        humidity = data.get('humidity')

        if temperature is None or humidity is None:
            return jsonify({'error': 'Temperature and humidity are required'}), 400

        response = {
            'message': 'Data received successfully',
            'temperature': temperature,
            'humidity': humidity
        }

        return jsonify(response), 200
    else:
        return jsonify({'error': 'Invalid data format, JSON required'}), 400

@app.route('/')
def home():
    return ('<h1> Home page</h1>')

if __name__ == '__main__':
    app.run(debug=True)
