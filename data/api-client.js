// API Communication Functions
// Handles all HTTP requests to the ESP32 server

function loadSystemInfo() {
    addDebugMessage('Loading system information...', 'info');
    fetch('/api/info')
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            systemInfo = data;
            document.getElementById('systemInfo').innerHTML = `
                <strong>Detected Boards:</strong> ${data.boardCount}<br>
                <strong>Servos per Board:</strong> ${data.servosPerBoard}<br>
                <strong>Total Servos:</strong> ${data.boardCount * data.servosPerBoard}
            `;
            addDebugMessage(`System info loaded: ${data.boardCount} boards, ${data.boardCount * data.servosPerBoard} total servos`, 'success');
        } else {
            showStatus('Failed to load system information', true);
        }
    })
    .catch(error => {
        showStatus(`Error loading system info: ${error.message}`, true);
    });
}

function loadConfiguration() {
    addDebugMessage('Loading configuration...', 'info');
    fetch('/api/config')
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            configuration = data;
            renderConfiguration();
            showStatus('Configuration loaded successfully');
        } else {
            showStatus('Failed to load configuration', true);
        }
    })
    .catch(error => {
        showStatus(`Error loading configuration: ${error.message}`, true);
    });
}

function updateServoConfig(boardIndex, servoIndex, field, value) {
    const data = {
        board: boardIndex,
        servo: servoIndex,
        field: field,
        value: String(value)
    };
    
    console.log(`DEBUG: updateServoConfig called - Board ${boardIndex}, Servo ${servoIndex}, Field: ${field}, Value: ${value}`);
    
    fetch('/api/config', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(data)
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showStatus(`Updated ${field} for Board ${boardIndex} Servo ${servoIndex}`);
            // Update local configuration
            if (configuration.boards && configuration.boards[boardIndex] && 
                configuration.boards[boardIndex].servos && configuration.boards[boardIndex].servos[servoIndex]) {
                configuration.boards[boardIndex].servos[servoIndex][field] = value;
            }
            
            // If isPairMaster was updated, trigger a re-render to update slave visibility
            if (field === 'isPairMaster') {
                console.log(`DEBUG: isPairMaster updated to ${value}, triggering re-render`);
                setTimeout(() => {
                    renderConfiguration();
                }, 500);
            }
        } else {
            showStatus(`Error updating ${field}: ${data.message}`, true);
        }
    })
    .catch(error => {
        showStatus(`Network error: ${error.message}`, true);
    });
}

function saveOfflineConfiguration() {
    addDebugMessage('Saving configuration to offline storage...', 'info');
    fetch('/api/save-offline', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showStatus('Configuration saved to offline storage');
        } else {
            showStatus(`Failed to save offline configuration: ${data.message}`, true);
        }
    })
    .catch(error => {
        showStatus(`Error saving offline configuration: ${error.message}`, true);
    });
}

function loadOfflineConfiguration() {
    addDebugMessage('Loading configuration from offline storage...', 'info');
    fetch('/api/load-offline', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showStatus('Configuration loaded from offline storage');
            // Reload the configuration to reflect changes
            loadConfiguration();
        } else {
            showStatus(`Failed to load offline configuration: ${data.message}`, true);
        }
    })
    .catch(error => {
        showStatus(`Error loading offline configuration: ${error.message}`, true);
    });
}

function startDebugPolling() {
    if (debugPollingInterval) {
        clearInterval(debugPollingInterval);
    }
    
    debugPollingInterval = setInterval(() => {
        fetch('/api/debug')
        .then(response => response.json())
        .then(data => {
            if (data.messages && data.messages.length > 0) {
                const debugMessages = document.getElementById('debugMessages');
                debugMessages.innerHTML = '';
                
                data.messages.forEach(msg => {
                    const messageDiv = document.createElement('div');
                    messageDiv.className = `debug-message ${msg.type}`;
                    messageDiv.innerHTML = `
                        <span class="debug-timestamp">${msg.timestamp}</span><br>
                        ${msg.message}
                    `;
                    debugMessages.appendChild(messageDiv);
                });
                
                debugMessages.scrollTop = debugMessages.scrollHeight;
            }
        })
        .catch(error => {
            console.error('Debug polling error:', error);
        });
    }, 1000); // Poll every second
}

function clearDebugConsole() {
    fetch('/api/debug/clear', { method: 'POST' })
    .then(() => {
        document.getElementById('debugMessages').innerHTML = '';
    });
}