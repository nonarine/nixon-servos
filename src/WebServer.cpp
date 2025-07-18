#include "WebServer.h"
#include <ArduinoJson.h>

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
}

void WebServerManager::setupRoutes() {
  // Serve static files from LittleFS
  server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  
  // Add error handling
  server->onNotFound([this](AsyncWebServerRequest *request) {
    this->handleNotFound(request);
  });
  
  // API endpoints
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
  
  server->on("/api/debug", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->handleGetDebug(request);
  });
  
  server->on("/api/debug/clear", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->handleClearDebug(request);
  });
  
  server->on("/api/command", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      this->handleCommand(request, data, len, index, total);
    });
  
  // Script management endpoints
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
  
  server->on("/api/scripts/execute", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      this->handleExecuteScript(request, data, len, index, total);
    });
}

void WebServerManager::handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void WebServerManager::handleGetInfo(AsyncWebServerRequest *request) {
  String response = servoController->getSystemInfoJson();
  request->send(200, "application/json", response);
}

void WebServerManager::handleGetConfig(AsyncWebServerRequest *request) {
  String response = servoController->getConfigurationJson();
  request->send(200, "application/json", response);
}

void WebServerManager::handlePostConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  String body = String((char*)data).substring(0, len);
  JsonDocument doc;
  deserializeJson(doc, body);
  
  if (doc["board"].is<int>() && doc["servo"].is<int>()) {
    int boardIndex = doc["board"];
    int servoIndex = doc["servo"];
    
    bool success = false;
    String message = "Invalid parameters";
    
    if (boardIndex >= 0 && boardIndex < servoController->getDetectedBoardCount() && 
        servoIndex >= 0 && servoIndex < SERVOS_PER_BOARD) {
      
      // Update each field that was provided
      for (JsonPair kv : doc.as<JsonObject>()) {
        String key = kv.key().c_str();
        if (key != "board" && key != "servo") {
          String value;
          if (kv.value().is<bool>()) {
            value = kv.value().as<bool>() ? "true" : "false";
          } else if (kv.value().is<int>()) {
            value = String(kv.value().as<int>());
          } else if (kv.value().is<float>()) {
            value = String(kv.value().as<float>());
          } else if (kv.value().is<const char*>()) {
            value = kv.value().as<const char*>();
          }
          
          if (servoController->updateServoConfig(boardIndex, servoIndex, key, value)) {
            success = true;
            message = "Configuration updated";
            DebugConsole::getInstance().logf("info", "Updated %s for Board %d Servo %d", key.c_str(), boardIndex, servoIndex);
          }
        }
      }
      
      if (success) {
        servoController->saveConfiguration();
      }
    }
    
    JsonDocument responseDoc;
    responseDoc["success"] = success;
    responseDoc["message"] = message;
    
    String response;
    serializeJson(responseDoc, response);
    request->send(success ? 200 : 400, "application/json", response);
  } else {
    String response = "{\"success\":false,\"message\":\"Missing board or servo parameter\"}";
    request->send(400, "application/json", response);
  }
}

void WebServerManager::handleTestServo(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  String body = String((char*)data).substring(0, len);
  JsonDocument doc;
  deserializeJson(doc, body);
  
  if (doc["board"].is<int>() && doc["servo"].is<int>() && doc["position"].is<float>()) {
    int boardIndex = doc["board"];
    int servoIndex = doc["servo"];
    float position = doc["position"];
    
    if (boardIndex >= 0 && boardIndex < servoController->getDetectedBoardCount() && 
        servoIndex >= 0 && servoIndex < SERVOS_PER_BOARD) {
      
      servoController->setServoToConfiguredPosition(boardIndex, servoIndex, position);
      DebugConsole::getInstance().logf("info", "Test position %.1f%% applied to Board %d Servo %d", position, boardIndex, servoIndex);
      String response = "{\"success\":true,\"message\":\"Servo test position set\"}";
      request->send(200, "application/json", response);
    } else {
      String response = "{\"success\":false,\"message\":\"Invalid board or servo index\"}";
      request->send(400, "application/json", response);
    }
  } else {
    String response = "{\"success\":false,\"message\":\"Missing board, servo, or position parameter\"}";
    request->send(400, "application/json", response);
  }
}

void WebServerManager::handleInitServos(AsyncWebServerRequest *request) {
  servoController->applyInitialPositions();
  DebugConsole::getInstance().log("Initial positions applied to all enabled servos", "success");
  String response = "{\"success\":true,\"message\":\"Initial positions applied\"}";
  request->send(200, "application/json", response);
}

void WebServerManager::handleSaveOffline(AsyncWebServerRequest *request) {
  servoController->saveOfflineConfiguration();
  String response = "{\"success\":true,\"message\":\"Configuration saved to offline storage\"}";
  request->send(200, "application/json", response);
}

void WebServerManager::handleLoadOffline(AsyncWebServerRequest *request) {
  servoController->loadOfflineConfiguration();
  String response = "{\"success\":true,\"message\":\"Configuration loaded from offline storage\"}";
  request->send(200, "application/json", response);
}

void WebServerManager::handleGetDebug(AsyncWebServerRequest *request) {
  String response = DebugConsole::getInstance().getMessagesJson();
  request->send(200, "application/json", response);
}

void WebServerManager::handleClearDebug(AsyncWebServerRequest *request) {
  DebugConsole::getInstance().clear();
  String response = "{\"success\":true,\"message\":\"Debug console cleared\"}";
  request->send(200, "application/json", response);
}

void WebServerManager::handleCommand(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  String body = String((char*)data).substring(0, len);
  JsonDocument doc;
  deserializeJson(doc, body);
  
  if (doc["command"].is<String>()) {
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

// Script Management Handlers
void WebServerManager::handleGetScripts(AsyncWebServerRequest *request) {
  String response = servoController->getScriptsJson();
  request->send(200, "application/json", response);
}

void WebServerManager::handlePostScript(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  String body = String((char*)data).substring(0, len);
  JsonDocument doc;
  deserializeJson(doc, body);
  
  if (doc["name"].is<String>() && doc["description"].is<String>() && doc["commands"].is<String>()) {
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
  String body = String((char*)data).substring(0, len);
  JsonDocument doc;
  deserializeJson(doc, body);
  
  if (doc["index"].is<int>() && doc["name"].is<String>() && doc["description"].is<String>() && doc["commands"].is<String>()) {
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
  String body = String((char*)data).substring(0, len);
  JsonDocument doc;
  deserializeJson(doc, body);
  
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
  String body = String((char*)data).substring(0, len);
  JsonDocument doc;
  deserializeJson(doc, body);
  
  if (doc["name"].is<String>()) {
    String scriptName = doc["name"];
    
    if (servoController->executeScript(scriptName)) {
      DebugConsole::getInstance().log("Script executed: " + scriptName, "info");
      String response = "{\"success\":true,\"message\":\"Script executed successfully\"}";
      request->send(200, "application/json", response);
    } else {
      String response = "{\"success\":false,\"message\":\"Failed to execute script. Script may not exist or be disabled.\"}";
      request->send(400, "application/json", response);
    }
  } else if (doc["index"].is<int>()) {
    int scriptIndex = doc["index"];
    
    if (servoController->executeScript(scriptIndex)) {
      DebugConsole::getInstance().log("Script executed by index: " + String(scriptIndex), "info");
      String response = "{\"success\":true,\"message\":\"Script executed successfully\"}";
      request->send(200, "application/json", response);
    } else {
      String response = "{\"success\":false,\"message\":\"Failed to execute script. Index may be invalid or script disabled.\"}";
      request->send(400, "application/json", response);
    }
  } else {
    String response = "{\"success\":false,\"message\":\"Invalid format. Expected: {\\\"name\\\": \\\"...\\\"} or {\\\"index\\\": 0}\"}";
    request->send(400, "application/json", response);
  }
}