var socket = io.connect('http://' + document.domain + ':' + location.port)

socket.on('update_temperature', function(data) {
    document.getElementById("temperature").innerText = "Temperature: " + (data.temperature !== null ? data.temperature : "No data available")
    document.getElementById("humidity").innerText = "Humidity: " + (data.humidity !== null ? data.humidity : "No data available")
})

socket.on('sensor_unavailable', function(data) {
    document.body.innerHTML = `
        <div class="container">
            <h1>500 - Internal Server Error</h1>
            <p>The sensor is currently unavailable. Please try again later.</p>
        </div>
    `
})

window.onload = function() {
    socket.emit('connect')
}