// Main Application Script
// Global state and initialization

let systemInfo = {};
let configuration = {};
let debugPollingInterval = null;

// Initialize the page when DOM is loaded
document.addEventListener('DOMContentLoaded', function() {
    initializePage();
});