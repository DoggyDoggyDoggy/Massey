from flask import Flask, jsonify, render_template, request, redirect, url_for
from sense_emu import SenseHat
from flask_sqlalchemy import SQLAlchemy
import datetime

from config import thresholds

app = Flask(__name__)
sense = SenseHat()

app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///sensor_data.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

db = SQLAlchemy(app)

warnings = []


class SensorData(db.Model):
    id = db.Column(db.Integer, primary_key = True)
    timestamp = db.Column(db.DateTime, default = datetime.datetime.utcnow)
    temperature = db.Column(db.Float, nullable = False)
    humidity = db.Column(db.Float, nullable = False)
    pressure = db.Column(db.Float, nullable = False)

    def __repr__(self):
        return f'<SensorData {self.id}>'


with app.app_context():
    db.create_all()


def check_data(temperature, humidity, pressure):
    check_temperature(temperature)
    check_humidity(humidity)
    check_pressure(pressure)


def check_temperature(temperature):
    if temperature > thresholds["temperature"]["max"]:
        warnings.append("High Temp!")
        sense.show_message("High Temp!", text_colour = [255, 0, 0])
    elif temperature < thresholds["temperature"]["min"]:
        warnings.append("Low Temp!")
        sense.show_message("Low Temp!", text_colour = [255, 0, 0])


def check_humidity(humidity):
    if humidity > thresholds["humidity"]["max"]:
        warnings.append("High Humidity!")
        sense.show_message("High Humidity!", text_colour = [255, 0, 0])
    elif humidity < thresholds["humidity"]["min"]:
        warnings.append("Low Humidity!")
        sense.show_message("Low Humidity!", text_colour = [255, 0, 0])


def check_pressure(pressure):
    if pressure > thresholds["pressure"]["max"]:
        warnings.append("High Pressure!")
        sense.show_message("High Pressure!", text_colour = [255, 0, 0])
    elif pressure < thresholds["pressure"]["min"]:
        warnings.append("Low Pressure!")
        sense.show_message("Low Pressure!", text_colour = [255, 0, 0])


def data_validation_from_sensor(temperature, humidity, pressure):
    if not (-35 <= temperature <= 105):
        raise ValueError("Received implausible data values. Available range from -35 to 105")
    if not (0 <= humidity <= 100):
        raise ValueError("Received implausible data values. Available range from 0 to 100")
    if not (260 <= pressure <= 1260):
        raise ValueError("Received implausible data values. Available range from 260 to 1260")


@app.route('/temperature')
def get_temperature():
    global warnings
    try:
        temperature = sense.get_temperature()
        humidity = sense.get_humidity()
        pressure = sense.get_pressure()

        warnings = []

        data_validation_from_sensor(temperature, humidity, pressure)

        check_data(temperature, humidity, pressure)

        sensor_data = SensorData(
            temperature = temperature,
            humidity = humidity,
            pressure = pressure
        )
        db.session.add(sensor_data)
        db.session.commit()

        return jsonify(temperature = temperature, humidity = humidity, pressure = pressure, warnings = warnings)
    except Exception as e:
        return jsonify(error = str(e)), 500


@app.route('/api/historical-data')
def historical_data():
    sensor_records = SensorData.query.order_by(SensorData.timestamp).all()
    data = [
        {
            "timestamp": record.timestamp.isoformat(),
            "temperature": record.temperature
        }
        for record in sensor_records
    ]
    return jsonify(data = data)


@app.route('/historical')
def historical():
    return render_template('historical.html')


@app.route('/settings', methods = ['GET', 'POST'])
def settings():
    if request.method == 'POST':
        try:
            thresholds["temperature"]["min"] = float(request.form.get("temp_min"))
            thresholds["temperature"]["max"] = float(request.form.get("temp_max"))
            thresholds["humidity"]["min"] = float(request.form.get("hum_min"))
            thresholds["humidity"]["max"] = float(request.form.get("hum_max"))
            thresholds["pressure"]["min"] = float(request.form.get("pres_min"))
            thresholds["pressure"]["max"] = float(request.form.get("pres_max"))
        except (TypeError, ValueError):
            return "Wrong data", 400
        return redirect(url_for('settings'))
    return render_template('settings.html', thresholds = thresholds)


@app.route('/')
def home():
    return render_template('HTMLEmuData.html')


if __name__ == '__main__':
    app.run(debug = True, port = 5001)
