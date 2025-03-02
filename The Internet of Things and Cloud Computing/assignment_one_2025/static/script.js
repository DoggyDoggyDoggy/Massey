function updateTemperatureDisplay(data) {
  // Fetch elements by their IDs to display temperature and warnings
  const tempDisplay = document.getElementById('temperature');
  const humidityDisplay = document.getElementById('humidity');
  const warningDisplay = document.getElementById('temp-warning');

  // Update text content of temperature display with the data received
  tempDisplay.textContent = `Current temperature: ${data.temperature}`;
  humidityDisplay.textContent = `Current humidity: ${data.humidity}`;
  warningDisplay.innerHTML = ''; // Clear any previous warnings

  // Check if there are any warnings in the data and display them
  if (data.warnings.length > 0) {
    data.warnings.forEach(function(warning) {
      // Append warnings in red color to the warning display element
      warningDisplay.innerHTML += `<div style="color: red;"><strong>${warning}</strong></div>`;
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
      document.getElementById('temp-warning').innerHTML = ''; // Clear any previous warnings
    });
}

// Execute fetchTemperatureData when the page loads and set it to repeat every 5 seconds
window.onload = function() {
  fetchTemperatureData(); // Fetch temperature data immediately on page load
  setInterval(fetchTemperatureData, 5000); // Continuously fetch data every 5 seconds
};
