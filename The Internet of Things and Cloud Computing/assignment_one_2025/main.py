from flask import Flask, jsonify, render_template
from sense_emu import SenseHat

app = Flask(__name__)
sense = SenseHat()

warnings = []


def check_data(temperature, humidity, pressure):
    check_temperature(temperature)
    check_humidity(humidity)
    check_pressure(pressure)


def check_temperature(temperature):
    if temperature > 40:
        warnings.append("High Temp!")
        sense.show_message("High Temp!", text_colour = [255, 0, 0])
    elif temperature < -4:
        warnings.append("Low Temp!")
        sense.show_message("Low Temp!", text_colour = [255, 0, 0])


def check_humidity(humidity):
    if humidity > 90:
        warnings.append("High Humidity!")
        sense.show_message("High Humidity!", text_colour = [255, 0, 0])
    elif humidity < 10:
        warnings.append("Low Humidity!")
        sense.show_message("High Humidity!", text_colour = [255, 0, 0])


def check_pressure(pressure):
    if pressure > 1030:
        warnings.append("High Pressure!")
        sense.show_message("High Pressure!", text_colour = [255, 0, 0])
    elif pressure < 970:
        warnings.append("Low Pressure!")
        sense.show_message("High Pressure!", text_colour = [255, 0, 0])


@app.route('/temperature')
def get_temperature():
    global warnings
    try:
        temperature = sense.get_temperature()
        humidity = sense.get_humidity()
        pressure = sense.get_pressure()

        warnings = []

        check_data(temperature, humidity, pressure)

        if not (-12 <= temperature <= 50):
            raise ValueError("Received implausible data values.")

        return jsonify(temperature = temperature, humidity = humidity, pressure = pressure, warnings = warnings)
    except Exception as e:
        return jsonify(error = str(e)), 500


@app.route('/')
def home():
    return render_template('HTMLEmuData.html')


if __name__ == '__main__':
    app.run(debug = True, port = 5001)
