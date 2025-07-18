// Servo Testing Functions
// Handles position testing, sliders, and test buttons

function testServoPosition(boardIndex, servoIndex, position) {
    const data = {
        board: boardIndex,
        servo: servoIndex,
        position: position
    };
    
    fetch('/api/test', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(data)
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showStatus(`Test position ${position.toFixed(1)}% applied to Board ${boardIndex} Servo ${servoIndex}`);
        } else {
            showStatus(`Test failed: ${data.message}`, true);
        }
    })
    .catch(error => {
        showStatus(`Test error: ${error.message}`, true);
    });
}

function updateSliderValue(boardIndex, servoIndex, value) {
    const valueSpan = document.getElementById(`sliderValue-${boardIndex}-${servoIndex}`);
    if (valueSpan) {
        valueSpan.textContent = parseFloat(value).toFixed(1) + '%';
    }
}

function setSliderAndTest(boardIndex, servoIndex, position) {
    const slider = document.getElementById(`testSlider-${boardIndex}-${servoIndex}`);
    if (slider) {
        slider.value = position;
        updateSliderValue(boardIndex, servoIndex, position);
    }
    testServoPosition(boardIndex, servoIndex, position);
}

function applyInitialPositions() {
    fetch('/api/init', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showStatus('Initial positions applied to all enabled servos');
        } else {
            showStatus(`Failed to apply initial positions: ${data.message}`, true);
        }
    })
    .catch(error => {
        showStatus(`Error applying initial positions: ${error.message}`, true);
    });
}