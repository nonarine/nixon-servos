#include "WebServer.h"
#include <ArduinoJson.h>

// ============================================================================
// SERVO CONTROL HANDLERS
// ============================================================================

void WebServerManager::handleGetInfo(AsyncWebServerRequest *request) {
  DebugConsole::getInstance().log("GET /api/info - System information", "info");
  String response = servoController->getSystemInfoJson();
  request->send(200, "application/json", response);
}

void WebServerManager::handleGetConfig(AsyncWebServerRequest *request) {
  DebugConsole::getInstance().log("GET /api/config - Configuration", "info");
  String response = servoController->getConfigurationJson();
  request->send(200, "application/json", response);
}

void WebServerManager::handlePostConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DebugConsole::getInstance().log("POST /api/config - Update configuration", "info");
  Serial.println("POST /api/config - Update configuration");
  
  String body = String((char*)data).substring(0, len);
  DebugConsole::getInstance().log("Config request body: " + body, "info");
  Serial.println("Config request body: " + body);
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    DebugConsole::getInstance().log("JSON parsing error in config update: " + String(error.c_str()), "error");
    Serial.println("JSON parsing error in config update: " + String(error.c_str()));
    String response = "{\"success\":false,\"message\":\"Invalid JSON format\"}";
    request->send(400, "application/json", response);
    return;
  }
  
  DebugConsole::getInstance().log("Parsed JSON successfully", "info");
  Serial.println("Parsed JSON successfully");
  
  // Debug all JSON fields
  DebugConsole::getInstance().logf("info", "JSON fields: board=%s, servo=%s, field=%s, value=%s", 
    doc["board"].is<int>() ? "int" : "missing/invalid",
    doc["servo"].is<int>() ? "int" : "missing/invalid", 
    doc["field"].is<const char*>() ? "string" : "missing/invalid",
    doc["value"].is<const char*>() ? "string" : "missing/invalid");
  Serial.printf("JSON fields: board=%s, servo=%s, field=%s, value=%s\n",
    doc["board"].is<int>() ? "int" : "missing/invalid",
    doc["servo"].is<int>() ? "int" : "missing/invalid", 
    doc["field"].is<const char*>() ? "string" : "missing/invalid",
    doc["value"].is<const char*>() ? "string" : "missing/invalid");
  
  // Debug actual values
  if (doc["board"].is<int>()) {
    DebugConsole::getInstance().logf("info", "Board value: %d", doc["board"].as<int>());
    Serial.printf("Board value: %d\n", doc["board"].as<int>());
  }
  if (doc["servo"].is<int>()) {
    DebugConsole::getInstance().logf("info", "Servo value: %d", doc["servo"].as<int>());
    Serial.printf("Servo value: %d\n", doc["servo"].as<int>());
  }
  if (doc["field"].is<const char*>()) {
    DebugConsole::getInstance().logf("info", "Field value: %s", doc["field"].as<const char*>());
    Serial.printf("Field value: %s\n", doc["field"].as<const char*>());
  }
  if (doc["value"].is<const char*>()) {
    DebugConsole::getInstance().logf("info", "Value: %s", doc["value"].as<const char*>());
    Serial.printf("Value: %s\n", doc["value"].as<const char*>());
  }
  
  if (doc["board"].is<int>() && doc["servo"].is<int>() && doc["field"].is<const char*>() && (doc["value"].is<const char*>() || doc["value"].is<bool>())) {
    int boardIndex = doc["board"];
    int servoIndex = doc["servo"];
    String field = doc["field"];
    String value;
    
    // Handle both string and boolean values
    if (doc["value"].is<const char*>()) {
      value = doc["value"].as<const char*>();
    } else if (doc["value"].is<bool>()) {
      value = doc["value"].as<bool>() ? "true" : "false";
    }
    
    if (servoController->updateServoConfig(boardIndex, servoIndex, field, value)) {
      servoController->saveConfiguration();
      String response = "{\"success\":true,\"message\":\"Configuration updated successfully\"}";
      request->send(200, "application/json", response);
    } else {
      String response = "{\"success\":false,\"message\":\"Failed to update configuration\"}";
      request->send(400, "application/json", response);
    }
  } else {
    String response = "{\"success\":false,\"message\":\"Invalid configuration format\"}";
    request->send(400, "application/json", response);
  }
}

void WebServerManager::handleTestServo(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DebugConsole::getInstance().log("POST /api/test - Test servo", "info");
  
  String body = String((char*)data).substring(0, len);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    DebugConsole::getInstance().log("JSON parsing error in servo test: " + String(error.c_str()), "error");
    String response = "{\"success\":false,\"message\":\"Invalid JSON format\"}";
    request->send(400, "application/json", response);
    return;
  }
  
  if (doc["board"].is<int>() && doc["servo"].is<int>() && doc["position"].is<float>()) {
    int boardIndex = doc["board"];
    int servoIndex = doc["servo"];
    float position = doc["position"];
    
    servoController->setServoToConfiguredPosition(boardIndex, servoIndex, position);
    
    String response = "{\"success\":true,\"message\":\"Servo position set successfully\"}";
    request->send(200, "application/json", response);
  } else {
    String response = "{\"success\":false,\"message\":\"Invalid servo test format\"}";
    request->send(400, "application/json", response);
  }
}

void WebServerManager::handleInitServos(AsyncWebServerRequest *request) {
  DebugConsole::getInstance().log("POST /api/init - Initialize servos", "info");
  
  servoController->applyInitialPositions();
  
  String response = "{\"success\":true,\"message\":\"Servos initialized to default positions\"}";
  request->send(200, "application/json", response);
}

void WebServerManager::handleSaveOffline(AsyncWebServerRequest *request) {
  DebugConsole::getInstance().log("POST /api/save-offline - Save offline config", "info");
  
  servoController->saveOfflineConfiguration();
  
  String response = "{\"success\":true,\"message\":\"Configuration saved to offline storage\"}";
  request->send(200, "application/json", response);
}

void WebServerManager::handleLoadOffline(AsyncWebServerRequest *request) {
  DebugConsole::getInstance().log("POST /api/load-offline - Load offline config", "info");
  
  servoController->loadOfflineConfiguration();
  
  String response = "{\"success\":true,\"message\":\"Configuration loaded from offline storage\"}";
  request->send(200, "application/json", response);
}