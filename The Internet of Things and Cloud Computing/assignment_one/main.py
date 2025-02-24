from flask import Flask, request, jsonify, render_template
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app)

current_temperature = None
current_humidity = None


@app.route('/api/data', methods = ['POST'])
def receive_data():
    global current_temperature
    global current_humidity

    if request.is_json:
        data = request.get_json()
        temperature = data.get('temperature')
        humidity = data.get('humidity')

        current_temperature = temperature
        current_humidity = humidity

        if temperature is None or humidity is None:
            return jsonify({'error': 'Temperature and humidity are required'}), 400

        response = {
            'message': 'Data received successfully',
            'temperature': temperature,
            'humidity': humidity
        }

        socketio.emit('update_temperature', {'temperature': temperature, 'humidity': humidity})

        return jsonify(response), 200
    else:
        return jsonify({'error': 'Invalid data format, JSON required'}), 400


@app.route('/')
def home():
    return render_template('main.html', temperature = current_temperature, humidity = current_humidity)


@socketio.on('connect')
def handle_connect():
    if current_temperature is not None:
        socketio.emit('update_temperature', {'temperature': current_temperature, 'humidity': current_humidity})


if __name__ == '__main__':
    socketio.run(app, debug = True, allow_unsafe_werkzeug = True)
