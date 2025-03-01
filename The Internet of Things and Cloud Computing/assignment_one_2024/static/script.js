var socket = io.connect('http://' + document.domain + ':' + location.port)
var isError = false

socket.on('update_temperature', function(data) {
    if (!isError) {
        document.getElementById("temperature").innerText = "Temperature: " + (data.temperature !== null ? data.temperature : "No data available")
        document.getElementById("humidity").innerText = "Humidity: " + (data.humidity !== null ? data.humidity : "No data available")

        if (data.temperature >40 && data.humidity > 90) {
                alert("Warning message!\nHumidity is over 90% and temperature is over 40°C")
                document.getElementById("warning").innerText = "Warning: Humidity is over 90% and temperature is over 40°C"
            }
        else if (data.humidity > 90) {
                alert("Warning message!\nHumidity is over 90%")
                document.getElementById("warning").innerText = "Warning: Humidity is over 90%"
            }
        else if (data.temperature >40) {
                alert("Warning message!\nTemperature is over 40°C")
                document.getElementById("warning").innerText = "Warning: Temperature is over 40°C"
            }
        else {
                document.getElementById("warning").innerText = ""
            }
    } else {
        window.location.reload()
    }
})

socket.on('sensor_unavailable', function(data) {
    isError = true
    document.body.innerHTML = `
        <div class="container">
            <h1>500 - Internal Server Error</h1>
            <p>The sensor is currently unavailable. Please try again later.</p>
        </div>
    `
})