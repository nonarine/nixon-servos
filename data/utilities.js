// Utility Functions
// Common helper functions used across the application

function showStatus(message, isError = false) {
    const debugMessages = document.getElementById('debugMessages');
    const messageDiv = document.createElement('div');
    messageDiv.className = `debug-message ${isError ? 'error' : 'success'}`;
    
    const timestamp = new Date().toLocaleTimeString();
    messageDiv.innerHTML = `
        <span class="debug-timestamp">${timestamp}</span><br>
        ${message}
    `;
    
    debugMessages.appendChild(messageDiv);
    debugMessages.scrollTop = debugMessages.scrollHeight;
    
    // Keep only last 50 messages
    while (debugMessages.children.length > 50) {
        debugMessages.removeChild(debugMessages.firstChild);
    }
}

function addDebugMessage(message, type = 'info') {
    const debugMessages = document.getElementById('debugMessages');
    const messageDiv = document.createElement('div');
    messageDiv.className = `debug-message ${type}`;
    
    const timestamp = new Date().toLocaleTimeString();
    messageDiv.innerHTML = `
        <span class="debug-timestamp">${timestamp}</span><br>
        ${message}
    `;
    
    debugMessages.appendChild(messageDiv);
    debugMessages.scrollTop = debugMessages.scrollHeight;
    
    // Keep only last 50 messages
    while (debugMessages.children.length > 50) {
        debugMessages.removeChild(debugMessages.firstChild);
    }
}

function renderConfiguration() {
    const container = document.getElementById('boardsContainer');
    container.innerHTML = '';
    
    configuration.boards.forEach(board => {
        const boardDiv = document.createElement('div');
        boardDiv.className = 'board-section';
        
        boardDiv.innerHTML = `
            <div class="board-header">
                <div class="board-title">${board.name}</div>
                <div class="board-address">Address: 0x${board.address.toString(16).toUpperCase()}</div>
            </div>
            <div class="servo-grid" id="board-${board.index}-servos">
                ${board.servos.map(servo => {
                    const component = new ServoConfigComponent(board.index, servo, configuration);
                    return component.render();
                }).join('')}
            </div>
        `;
        
        container.appendChild(boardDiv);
    });
}

function exportConfiguration() {
    const configJson = JSON.stringify(configuration, null, 2);
    const blob = new Blob([configJson], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'servo_config.json';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    addDebugMessage('Configuration exported as servo_config.json', 'success');
}

// Initialize the page
function initializePage() {
    addDebugMessage('Page loaded, initializing...', 'info');
    loadSystemInfo();
    loadConfiguration();
    startDebugPolling();
    initializeCommandInterface();
}