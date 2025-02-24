from flask import Flask, request, jsonify, render_template, abort
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app)

current_temperature = None
current_humidity = None
sensor_available = True


@app.route('/api/data', methods = ['POST'])
def receive_data():
    global current_temperature
    global current_humidity
    global sensor_available

    if request.is_json:
        data = request.get_json()
        temperature = data.get('temperature')
        humidity = data.get('humidity')
        available = data.get('available')

        if available is False:
            sensor_available = False
            socketio.emit('sensor_unavailable')
            return jsonify({'error': 'Sensor unavailable'}), 500

        if temperature is None or humidity is None:
            return jsonify({'error': 'Temperature and humidity are required'}), 400

        current_temperature = temperature
        current_humidity = humidity
        sensor_available = True

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
    if not sensor_available:
        abort(500)

    return render_template('main.html', temperature = current_temperature, humidity = current_humidity)


@app.errorhandler(500)
def handle_internal_server_error(e):
    return render_template('500.html'), 500


@socketio.on('connect')
def handle_connect():
    if current_temperature is not None:
        socketio.emit('update_temperature', {'temperature': current_temperature, 'humidity': current_humidity})


if __name__ == '__main__':
    socketio.run(app, debug = True, allow_unsafe_werkzeug = True)
