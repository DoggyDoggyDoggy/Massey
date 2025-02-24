var socket = io.connect('http://' + document.domain + ':' + location.port)

socket.on('update_temperature', function(data) {
    document.getElementById("temperature").innerText = "Temperature: " + (data.temperature !== null ? data.temperature : "No data available")
    document.getElementById("humidity").innerText = "Humidity: " + (data.humidity !== null ? data.humidity : "No data available")
})

window.onload = function() {
    socket.emit('connect')
}