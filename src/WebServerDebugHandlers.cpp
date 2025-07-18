#include "WebServer.h"
#include <ArduinoJson.h>

// ============================================================================
// DEBUG AND UTILITY HANDLERS
// ============================================================================

void WebServerManager::handleNotFound(AsyncWebServerRequest *request) {
  DebugConsole::getInstance().log("404 Not Found: " + request->url(), "error");
  
  String response = "{\"success\":false,\"message\":\"Endpoint not found\",\"url\":\"" + request->url() + "\"}";
  request->send(404, "application/json", response);
}

void WebServerManager::handleGetDebug(AsyncWebServerRequest *request) {
  String response = DebugConsole::getInstance().getMessagesJson();
  request->send(200, "application/json", response);
}

void WebServerManager::handleClearDebug(AsyncWebServerRequest *request) {
  DebugConsole::getInstance().log("DELETE /api/debug - Clear debug console", "info");
  
  DebugConsole::getInstance().clear();
  String response = "{\"success\":true,\"message\":\"Debug console cleared\"}";
  request->send(200, "application/json", response);
}

void WebServerManager::handleCommand(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DebugConsole::getInstance().log("POST /api/command - Execute command", "info");
  
  // Simple approach - just use the current chunk (works for most cases)
  String body = String((char*)data).substring(0, len);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    DebugConsole::getInstance().log("JSON parsing error in command: " + String(error.c_str()), "error");
    String response = "{\"success\":false,\"message\":\"Invalid JSON format\"}";
    request->send(400, "application/json", response);
    return;
  }
  
  if (doc["command"].is<const char*>()) {
    String command = doc["command"];
    
    // Execute the command
    String result = servoController->executeCommand(command);
    
    // Log the command and result to debug console
    DebugConsole::getInstance().log("Command executed: " + command, "info");
    DebugConsole::getInstance().log("Result: " + result, "info");
    
    // Create response
    JsonDocument response;
    response["success"] = true;
    response["command"] = command;
    response["result"] = result;
    
    String responseStr;
    serializeJson(response, responseStr);
    request->send(200, "application/json", responseStr);
  } else {
    String response = "{\"success\":false,\"message\":\"Invalid command format. Expected: {\\\"command\\\": \\\"your command here\\\"}\"}";
    request->send(400, "application/json", response);
  }
}