#include "WebServer.h"

// ============================================================================
// WEB SERVER MANAGER - CORE FUNCTIONALITY
// ============================================================================

WebServerManager::WebServerManager(ServoController* controller) : servoController(controller) {
  server = new AsyncWebServer(80);
}

WebServerManager::~WebServerManager() {
  delete server;
}

void WebServerManager::begin() {
  setupRoutes();
  server->begin();
  Serial.println("Web server started on port 80");
  DebugConsole::getInstance().log("Web server started on port 80", "success");
}

// Route setup is implemented in WebServerRoutes.cpp
// Handler implementations are split across:
// - WebServerServoHandlers.cpp (servo control endpoints)
// - WebServerScriptHandlers.cpp (script management endpoints) 
// - WebServerDebugHandlers.cpp (debug and utility endpoints)