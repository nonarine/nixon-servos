#include "WebServer.h"

// ============================================================================
// ROUTE SETUP AND MANAGEMENT
// ============================================================================

void WebServerManager::setupRoutes() {
  DebugConsole::getInstance().log("Setting up web server routes", "info");
  
  // Serve static files from LittleFS
  server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  
  // Add error handling
  server->onNotFound([this](AsyncWebServerRequest *request) {
    this->handleNotFound(request);
  });
  
  // ========================================================================
  // SERVO CONTROL ENDPOINTS
  // ========================================================================
  
  server->on("/api/info", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->handleGetInfo(request);
  });
  
  server->on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->handleGetConfig(request);
  });
  
  server->on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      this->handlePostConfig(request, data, len, index, total);
    });
  
  server->on("/api/test", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      this->handleTestServo(request, data, len, index, total);
    });
  
  server->on("/api/init", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->handleInitServos(request);
  });
  
  server->on("/api/save-offline", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->handleSaveOffline(request);
  });
  
  server->on("/api/load-offline", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->handleLoadOffline(request);
  });
  
  // ========================================================================
  // SCRIPT MANAGEMENT ENDPOINTS
  // ========================================================================
  
  server->on("/api/scripts", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->handleGetScripts(request);
  });
  
  server->on("/api/scripts", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      this->handlePostScript(request, data, len, index, total);
    });
  
  server->on("/api/scripts", HTTP_PUT, [](AsyncWebServerRequest *request){}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      this->handlePutScript(request, data, len, index, total);
    });
  
  server->on("/api/scripts", HTTP_DELETE, [](AsyncWebServerRequest *request){}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      this->handleDeleteScript(request, data, len, index, total);
    });
  
  server->on("/api/execute-script", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      this->handleExecuteScript(request, data, len, index, total);
    });
  
  // ========================================================================
  // DEBUG AND UTILITY ENDPOINTS
  // ========================================================================
  
  server->on("/api/debug", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->handleGetDebug(request);
  });
  
  server->on("/api/debug", HTTP_DELETE, [this](AsyncWebServerRequest *request) {
    this->handleClearDebug(request);
  });
  
  server->on("/api/debug/test", HTTP_GET, [this](AsyncWebServerRequest *request) {
    DebugConsole::getInstance().log("Debug test endpoint called", "info");
    Serial.println("Debug test endpoint called");
    String response = "{\"success\":true,\"message\":\"Debug test successful\"}";
    request->send(200, "application/json", response);
  });
  
  server->on("/api/command", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      this->handleCommand(request, data, len, index, total);
    });
  
  DebugConsole::getInstance().log("Web server routes configured successfully", "success");
}