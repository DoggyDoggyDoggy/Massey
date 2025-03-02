from flask import Flask, jsonify,render_template
from sense_emu import SenseHat

app = Flask(__name__)
sense = SenseHat()

@app.route('/temperature')
def get_temperature():
    try:
        temperature = sense.get_temperature()
        humidity = sense.get_humidity()
        pressure = sense.get_pressure()

        warnings = []

        if not (0 <= temperature <= 50):
            raise ValueError("Received implausible data values.")

        if temperature > 40 or temperature < 0:
            warnings.append("High Temp!")
            sense.show_message("High Temp!", text_colour=[255, 0, 0])

        return jsonify(temperature=temperature, humidity = humidity, pressure = pressure, warnings=warnings)
    except Exception as e:
        return jsonify(error=str(e)), 500

@app.route('/')
def home():
    return render_template('HTMLEmuData.html')

if __name__ == '__main__':
    app.run(debug = True, port = 5001)