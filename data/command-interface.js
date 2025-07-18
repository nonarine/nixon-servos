// Command Interface Functions
// Handles text command input and execution

function executeCommand(command) {
    if (!command || command.trim().length === 0) {
        addDebugMessage('Error: Empty command', 'error');
        return;
    }
    
    addDebugMessage('Executing command: ' + command, 'info');
    
    fetch('/api/command', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ command: command })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            addDebugMessage('Command Result: ' + data.result, 'success');
        } else {
            addDebugMessage('Command Error: ' + data.message, 'error');
        }
    })
    .catch(error => {
        addDebugMessage('Network Error: ' + error.message, 'error');
    });
}

function handleCommandInput(event) {
    if (event.key === 'Enter') {
        const command = event.target.value.trim();
        if (command.length > 0) {
            executeCommand(command);
            event.target.value = '';
        }
    }
}

function executeCommandFromButton() {
    const input = document.getElementById('commandInput');
    const command = input.value.trim();
    if (command.length > 0) {
        executeCommand(command);
        input.value = '';
    }
}

function showCommandHelp() {
    executeCommand('help');
}

function initializeCommandInterface() {
    const commandInput = document.getElementById('commandInput');
    if (commandInput) {
        commandInput.addEventListener('keydown', handleCommandInput);
    }
}