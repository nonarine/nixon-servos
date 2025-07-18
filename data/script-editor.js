// Script Editor Functions
// Handles script creation, editing, and management

let scripts = [];
let editingScriptIndex = -1;

// Load scripts from server
function loadScripts() {
    addDebugMessage('Loading scripts...', 'info');
    fetch('/api/scripts')
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            scripts = data.scripts || [];
            renderScripts();
            addDebugMessage(`Loaded ${scripts.length} scripts`, 'success');
        } else {
            addDebugMessage('Failed to load scripts', 'error');
        }
    })
    .catch(error => {
        addDebugMessage('Error loading scripts: ' + error.message, 'error');
    });
}

// Render scripts list
function renderScripts() {
    const container = document.getElementById('scriptsContainer');
    
    if (scripts.length === 0) {
        container.innerHTML = '<p class="no-scripts">No scripts found. Create your first script above!</p>';
        return;
    }
    
    container.innerHTML = scripts.map(script => `
        <div class="script-card" data-index="${script.index}">
            <div class="script-header">
                <h3>${script.name}</h3>
                <div class="script-actions">
                    <button class="btn btn-small btn-primary" onclick="executeScript('${script.name}')">Execute</button>
                    <button class="btn btn-small btn-warning" onclick="editScript(${script.index})">Edit</button>
                    <button class="btn btn-small btn-danger" onclick="deleteScript(${script.index})">Delete</button>
                </div>
            </div>
            <p class="script-description">${script.description}</p>
            <div class="script-commands">
                <strong>Commands:</strong>
                <code>${script.commands}</code>
            </div>
            <div class="script-status ${script.enabled ? 'enabled' : 'disabled'}">
                ${script.enabled ? 'Enabled' : 'Disabled'}
            </div>
        </div>
    `).join('');
}

// Handle form submission
function handleScriptSubmission(event) {
    event.preventDefault();
    
    const name = document.getElementById('scriptName').value.trim();
    const description = document.getElementById('scriptDescription').value.trim();
    const commands = document.getElementById('scriptCommands').value.trim();
    
    if (!name || !description || !commands) {
        addDebugMessage('Please fill in all fields', 'error');
        return;
    }
    
    const scriptData = {
        name: name,
        description: description,
        commands: commands
    };
    
    if (editingScriptIndex >= 0) {
        // Update existing script
        scriptData.index = editingScriptIndex;
        updateScript(scriptData);
    } else {
        // Create new script
        createScript(scriptData);
    }
}

// Create new script
function createScript(scriptData) {
    addDebugMessage('Creating script: ' + scriptData.name, 'info');
    
    fetch('/api/scripts', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(scriptData)
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            addDebugMessage('Script created successfully', 'success');
            clearForm();
            loadScripts();
        } else {
            addDebugMessage('Error creating script: ' + data.message, 'error');
        }
    })
    .catch(error => {
        addDebugMessage('Network error: ' + error.message, 'error');
    });
}

// Update existing script
function updateScript(scriptData) {
    addDebugMessage('Updating script: ' + scriptData.name, 'info');
    
    fetch('/api/scripts', {
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(scriptData)
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            addDebugMessage('Script updated successfully', 'success');
            clearForm();
            loadScripts();
        } else {
            addDebugMessage('Error updating script: ' + data.message, 'error');
        }
    })
    .catch(error => {
        addDebugMessage('Network error: ' + error.message, 'error');
    });
}

// Delete script
function deleteScript(index) {
    const script = scripts.find(s => s.index === index);
    if (!script) return;
    
    if (!confirm(`Are you sure you want to delete the script "${script.name}"?`)) {
        return;
    }
    
    addDebugMessage('Deleting script: ' + script.name, 'info');
    
    fetch('/api/scripts', {
        method: 'DELETE',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ index: index })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            addDebugMessage('Script deleted successfully', 'success');
            loadScripts();
        } else {
            addDebugMessage('Error deleting script: ' + data.message, 'error');
        }
    })
    .catch(error => {
        addDebugMessage('Network error: ' + error.message, 'error');
    });
}

// Execute script
function executeScript(scriptName) {
    addDebugMessage('Executing script: ' + scriptName, 'info');
    
    fetch('/api/scripts/execute', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ name: scriptName })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            addDebugMessage('Script executed successfully', 'success');
        } else {
            addDebugMessage('Error executing script: ' + data.message, 'error');
        }
    })
    .catch(error => {
        addDebugMessage('Network error: ' + error.message, 'error');
    });
}

// Edit script
function editScript(index) {
    const script = scripts.find(s => s.index === index);
    if (!script) return;
    
    // Fill form with script data
    document.getElementById('scriptName').value = script.name;
    document.getElementById('scriptDescription').value = script.description;
    document.getElementById('scriptCommands').value = script.commands;
    
    // Update UI to show editing mode
    editingScriptIndex = index;
    document.querySelector('.script-editor h2').textContent = `Edit Script: ${script.name}`;
    document.querySelector('button[type="submit"]').textContent = 'Update Script';
    
    // Scroll to form
    document.querySelector('.script-editor').scrollIntoView({ behavior: 'smooth' });
    
    addDebugMessage('Editing script: ' + script.name, 'info');
}

// Clear form
function clearForm() {
    document.getElementById('scriptForm').reset();
    editingScriptIndex = -1;
    document.querySelector('.script-editor h2').textContent = 'Create New Script';
    document.querySelector('button[type="submit"]').textContent = 'Save Script';
}

// Initialize script editor
function initializeScriptEditor() {
    // Set up form submission
    const form = document.getElementById('scriptForm');
    if (form) {
        form.addEventListener('submit', handleScriptSubmission);
    }
    
    // Load scripts
    loadScripts();
    
    addDebugMessage('Script editor initialized', 'info');
}

// Initialize when page loads
document.addEventListener('DOMContentLoaded', function() {
    initializeScriptEditor();
});