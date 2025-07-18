#include "WebServer.h"
#include <ArduinoJson.h>

// ============================================================================
// SCRIPT MANAGEMENT HANDLERS
// ============================================================================

void WebServerManager::handleGetScripts(AsyncWebServerRequest *request) {
  DebugConsole::getInstance().log("GET /api/scripts - Listing scripts", "info");
  String response = servoController->getScriptsJson();
  request->send(200, "application/json", response);
}

void WebServerManager::handlePostScript(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  // Log to both debug console and serial
  DebugConsole::getInstance().log("POST /api/scripts - Creating script", "info");
  Serial.println("POST /api/scripts - Creating script");
  
  // Simple approach - just use the current chunk (works for most cases)  
  String body = String((char*)data).substring(0, len);
  DebugConsole::getInstance().log("Create script request body: " + body, "info");
  Serial.println("Create script request body: " + body);
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    DebugConsole::getInstance().log("JSON parsing error in create script: " + String(error.c_str()), "error");
    String response = "{\"success\":false,\"message\":\"Invalid JSON format\"}";
    request->send(400, "application/json", response);
    return;
  }
  
  if (doc["name"].is<const char*>() && doc["description"].is<const char*>() && doc["commands"].is<const char*>()) {
    String name = doc["name"];
    String description = doc["description"];
    String commands = doc["commands"];
    
    if (servoController->addScript(name, description, commands)) {
      servoController->saveConfiguration();
      String response = "{\"success\":true,\"message\":\"Script added successfully\"}";
      request->send(200, "application/json", response);
    } else {
      String response = "{\"success\":false,\"message\":\"Failed to add script. Name may already exist or script limit reached.\"}";
      request->send(400, "application/json", response);
    }
  } else {
    String response = "{\"success\":false,\"message\":\"Invalid script format. Expected: {\\\"name\\\": \\\"...\\\", \\\"description\\\": \\\"...\\\", \\\"commands\\\": \\\"...\\\"}\"}";
    request->send(400, "application/json", response);
  }
}

void WebServerManager::handlePutScript(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DebugConsole::getInstance().log("PUT /api/scripts - Updating script", "info");
  
  // Simple approach - just use the current chunk (works for most cases)
  String body = String((char*)data).substring(0, len);
  DebugConsole::getInstance().log("Update script request body: " + body, "info");
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    DebugConsole::getInstance().log("JSON parsing error in update script: " + String(error.c_str()), "error");
    String response = "{\"success\":false,\"message\":\"Invalid JSON format\"}";
    request->send(400, "application/json", response);
    return;
  }
  
  if (doc["index"].is<int>() && doc["name"].is<const char*>() && doc["description"].is<const char*>() && doc["commands"].is<const char*>()) {
    int scriptIndex = doc["index"];
    String name = doc["name"];
    String description = doc["description"];
    String commands = doc["commands"];
    
    if (servoController->updateScript(scriptIndex, name, description, commands)) {
      servoController->saveConfiguration();
      String response = "{\"success\":true,\"message\":\"Script updated successfully\"}";
      request->send(200, "application/json", response);
    } else {
      String response = "{\"success\":false,\"message\":\"Failed to update script. Index may be invalid or name already exists.\"}";
      request->send(400, "application/json", response);
    }
  } else {
    String response = "{\"success\":false,\"message\":\"Invalid script format. Expected: {\\\"index\\\": 0, \\\"name\\\": \\\"...\\\", \\\"description\\\": \\\"...\\\", \\\"commands\\\": \\\"...\\\"}\"}";
    request->send(400, "application/json", response);
  }
}

void WebServerManager::handleDeleteScript(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  DebugConsole::getInstance().log("DELETE /api/scripts - Deleting script", "info");
  
  // Simple approach - just use the current chunk (works for most cases)
  String body = String((char*)data).substring(0, len);
  DebugConsole::getInstance().log("Delete script request body: " + body, "info");
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    DebugConsole::getInstance().log("JSON parsing error in delete script: " + String(error.c_str()), "error");
    String response = "{\"success\":false,\"message\":\"Invalid JSON format\"}";
    request->send(400, "application/json", response);
    return;
  }
  
  if (doc["index"].is<int>()) {
    int scriptIndex = doc["index"];
    
    if (servoController->deleteScript(scriptIndex)) {
      servoController->saveConfiguration();
      String response = "{\"success\":true,\"message\":\"Script deleted successfully\"}";
      request->send(200, "application/json", response);
    } else {
      String response = "{\"success\":false,\"message\":\"Failed to delete script. Index may be invalid.\"}";
      request->send(400, "application/json", response);
    }
  } else {
    String response = "{\"success\":false,\"message\":\"Invalid format. Expected: {\\\"index\\\": 0}\"}";
    request->send(400, "application/json", response);
  }
}

void WebServerManager::handleExecuteScript(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  // Log to both debug console and serial
  DebugConsole::getInstance().log("POST /api/execute-script - Executing script", "info");
  Serial.println("POST /api/execute-script - Executing script");
  
  // Simple approach - just use the current chunk (works for most cases)
  String body = String((char*)data).substring(0, len);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  
  // Debug logging
  DebugConsole::getInstance().log("Execute script request body: " + body, "info");
  Serial.println("Execute script request body: " + body);
  
  if (error) {
    DebugConsole::getInstance().log("JSON parsing error: " + String(error.c_str()), "error");
    String response = "{\"success\":false,\"message\":\"Invalid JSON format\"}";
    request->send(400, "application/json", response);
    return;
  }
  
  if (doc["name"].is<const char*>()) {
    String scriptName = doc["name"];
    DebugConsole::getInstance().log("Executing script by name: " + scriptName, "info");
    
    if (servoController->executeScript(scriptName)) {
      DebugConsole::getInstance().log("Script executed successfully: " + scriptName, "info");
      String response = "{\"success\":true,\"message\":\"Script executed successfully\"}";
      request->send(200, "application/json", response);
    } else {
      DebugConsole::getInstance().log("Script execution failed: " + scriptName, "error");
      String response = "{\"success\":false,\"message\":\"Failed to execute script. Script may not exist or be disabled.\"}";
      request->send(400, "application/json", response);
    }
  } else if (doc["index"].is<int>()) {
    int scriptIndex = doc["index"];
    DebugConsole::getInstance().log("Executing script by index: " + String(scriptIndex), "info");
    
    if (servoController->executeScript(scriptIndex)) {
      DebugConsole::getInstance().log("Script executed successfully by index: " + String(scriptIndex), "info");
      String response = "{\"success\":true,\"message\":\"Script executed successfully\"}";
      request->send(200, "application/json", response);
    } else {
      DebugConsole::getInstance().log("Script execution failed by index: " + String(scriptIndex), "error");
      String response = "{\"success\":false,\"message\":\"Failed to execute script. Index may be invalid or script disabled.\"}";
      request->send(400, "application/json", response);
    }
  } else {
    DebugConsole::getInstance().log("Invalid script execute request format", "error");
    DebugConsole::getInstance().log("Received JSON keys:", "error");
    for (JsonPair kv : doc.as<JsonObject>()) {
      DebugConsole::getInstance().log("  " + String(kv.key().c_str()) + ": " + kv.value().as<String>(), "error");
    }
    String response = "{\"success\":false,\"message\":\"Invalid execute format. Expected: {\\\"name\\\": \\\"script_name\\\"} or {\\\"index\\\": 0}\"}";
    request->send(400, "application/json", response);
  }
}