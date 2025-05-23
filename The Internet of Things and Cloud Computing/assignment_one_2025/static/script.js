//This script file were taken from the example that was shown in the second week
// I expanded it to show humidity and air pressure.
// The trend variable has been added

function updateTemperatureDisplay(data) {
  const tempDisplay = document.getElementById('temperature');
  const humidityDisplay = document.getElementById('humidity');
  const pressureDisplay = document.getElementById('pressure');
  const warningDisplay = document.getElementById('temp-warning');
  const trendDisplay = document.getElementById('trend');

  tempDisplay.textContent = `Current temperature: ${data.temperature}`;
  humidityDisplay.textContent = `Current humidity: ${data.humidity}`;
  pressureDisplay.textContent = `Current pressure: ${data.pressure}`;
  trendDisplay.textContent = `${data.trend}`;
  warningDisplay.innerHTML = '';

  if (data.warnings.length > 0) {
    data.warnings.forEach(function(warning) {
     warningDisplay.innerHTML +=  `<div style = 'color: red;'><strong>${warning}</strong></div>`;
    });
  }
}

// Function to fetch temperature data from the Flask server
function fetchTemperatureData() {
  fetch('/temperature') // Make a request to the '/temperature' endpoint
    .then(response => response.json()) // Parse the JSON response
    .then(data => {
      updateTemperatureDisplay(data); // Update the webpage with the fetched data
    })
    .catch(error => {
      // Log and display error if fetching temperature data fails
      console.error('Error fetching temperature:', error);
      document.getElementById('temperature').textContent = "Error loading data!";
      document.getElementById('humidity').textContent = "Error loading data!";
      document.getElementById('pressure').textContent = "Error loading data!";
      document.getElementById('temp-warning').innerHTML = ''; // Clear any previous warnings
    });
}

// Execute fetchTemperatureData when the page loads and set it to repeat every 5 seconds
window.onload = function() {
  fetchTemperatureData(); // Fetch temperature data immediately on page load
  setInterval(fetchTemperatureData, 5000); // Continuously fetch data every 5 seconds
};
