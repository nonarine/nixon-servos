#include "ServoController.h"

void ServoController::saveConfiguration() {
  JsonDocument doc;
  JsonArray boardsArray = doc["boards"].to<JsonArray>();
  
  for (int b = 0; b < detectedBoardCount; b++) {
    JsonObject boardObj = boardsArray.add<JsonObject>();
    boardObj["address"] = boards[b].address;
    boardObj["name"] = boards[b].name;
    boardObj["enabled"] = boards[b].enabled;
    
    JsonArray servosArray = boardObj["servos"].to<JsonArray>();
    for (int s = 0; s < SERVOS_PER_BOARD; s++) {
      JsonObject servoObj = servosArray.add<JsonObject>();
      servoObj["enabled"] = servoConfigs[b][s].enabled;
      servoObj["center"] = servoConfigs[b][s].center;
      servoObj["range"] = servoConfigs[b][s].range;
      servoObj["initPosition"] = servoConfigs[b][s].initPosition;
      servoObj["isPair"] = servoConfigs[b][s].isPair;
      servoObj["pairBoard"] = servoConfigs[b][s].pairBoard;
      servoObj["pairServo"] = servoConfigs[b][s].pairServo;
      servoObj["isPairMaster"] = servoConfigs[b][s].isPairMaster;
      servoObj["name"] = servoConfigs[b][s].name;
    }
  }
  
  // Add scripts to configuration
  JsonArray scriptsArray = doc["scripts"].to<JsonArray>();
  for (int i = 0; i < scriptCount; i++) {
    JsonObject scriptObj = scriptsArray.add<JsonObject>();
    scriptObj["index"] = i;
    scriptObj["name"] = scriptActions[i].name;
    scriptObj["description"] = scriptActions[i].description;
    scriptObj["commands"] = scriptActions[i].commands;
    scriptObj["enabled"] = scriptActions[i].enabled;
  }
  
  File file = LittleFS.open(CONFIG_FILE, "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
    DebugConsole::getInstance().log("Configuration saved to " + String(CONFIG_FILE), "success");
  } else {
    DebugConsole::getInstance().log("Failed to save configuration", "error");
  }
}

void ServoController::loadConfiguration() {
  if (!LittleFS.exists(CONFIG_FILE)) {
    DebugConsole::getInstance().log("No configuration file found, using defaults", "info");
    return;
  }
  
  File file = LittleFS.open(CONFIG_FILE, "r");
  if (!file) {
    DebugConsole::getInstance().log("Failed to open configuration file", "error");
    return;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    DebugConsole::getInstance().log("Failed to parse configuration file", "error");
    return;
  }
  
  JsonArray boardsArray = doc["boards"].as<JsonArray>();
  for (int b = 0; b < boardsArray.size() && b < detectedBoardCount; b++) {
    JsonObject boardObj = boardsArray[b];
    if (boardObj["address"] == boards[b].address) {
      strncpy(boards[b].name, boardObj["name"] | boards[b].name, sizeof(boards[b].name));
      boards[b].enabled = boardObj["enabled"] | boards[b].enabled;
      
      JsonArray servosArray = boardObj["servos"].as<JsonArray>();
      for (int s = 0; s < servosArray.size() && s < SERVOS_PER_BOARD; s++) {
        JsonObject servoObj = servosArray[s];
        servoConfigs[b][s].enabled = servoObj["enabled"];
        servoConfigs[b][s].center = servoObj["center"];
        servoConfigs[b][s].range = servoObj["range"];
        servoConfigs[b][s].initPosition = servoObj["initPosition"];
        servoConfigs[b][s].isPair = servoObj["isPair"];
        servoConfigs[b][s].pairBoard = servoObj["pairBoard"];
        servoConfigs[b][s].pairServo = servoObj["pairServo"];
        servoConfigs[b][s].isPairMaster = servoObj["isPairMaster"];
        strncpy(servoConfigs[b][s].name, servoObj["name"] | servoConfigs[b][s].name, sizeof(servoConfigs[b][s].name));
      }
    }
  }
  
  // Load scripts if they exist
  if (doc["scripts"].is<JsonArray>()) {
    JsonArray scriptsArray = doc["scripts"].as<JsonArray>();
    scriptCount = 0;
    for (int i = 0; i < scriptsArray.size() && i < MAX_SCRIPTS; i++) {
      JsonObject scriptObj = scriptsArray[i];
      strncpy(scriptActions[scriptCount].name, scriptObj["name"] | "", sizeof(scriptActions[scriptCount].name) - 1);
      scriptActions[scriptCount].name[sizeof(scriptActions[scriptCount].name) - 1] = '\0';
      
      strncpy(scriptActions[scriptCount].description, scriptObj["description"] | "", sizeof(scriptActions[scriptCount].description) - 1);
      scriptActions[scriptCount].description[sizeof(scriptActions[scriptCount].description) - 1] = '\0';
      
      strncpy(scriptActions[scriptCount].commands, scriptObj["commands"] | "", sizeof(scriptActions[scriptCount].commands) - 1);
      scriptActions[scriptCount].commands[sizeof(scriptActions[scriptCount].commands) - 1] = '\0';
      
      scriptActions[scriptCount].enabled = scriptObj["enabled"] | true;
      scriptCount++;
    }
  }
  
  DebugConsole::getInstance().log("Configuration loaded from " + String(CONFIG_FILE), "success");
}

void ServoController::saveOfflineConfiguration() {
  JsonDocument doc;
  JsonArray boardsArray = doc["boards"].to<JsonArray>();
  
  for (int b = 0; b < detectedBoardCount; b++) {
    JsonObject boardObj = boardsArray.add<JsonObject>();
    boardObj["address"] = boards[b].address;
    boardObj["name"] = boards[b].name;
    boardObj["enabled"] = boards[b].enabled;
    
    JsonArray servosArray = boardObj["servos"].to<JsonArray>();
    for (int s = 0; s < SERVOS_PER_BOARD; s++) {
      JsonObject servoObj = servosArray.add<JsonObject>();
      servoObj["enabled"] = servoConfigs[b][s].enabled;
      servoObj["center"] = servoConfigs[b][s].center;
      servoObj["range"] = servoConfigs[b][s].range;
      servoObj["initPosition"] = servoConfigs[b][s].initPosition;
      servoObj["isPair"] = servoConfigs[b][s].isPair;
      servoObj["pairBoard"] = servoConfigs[b][s].pairBoard;
      servoObj["pairServo"] = servoConfigs[b][s].pairServo;
      servoObj["isPairMaster"] = servoConfigs[b][s].isPairMaster;
      servoObj["name"] = servoConfigs[b][s].name;
    }
  }
  
  // Add scripts to offline configuration
  JsonArray scriptsArray = doc["scripts"].to<JsonArray>();
  for (int i = 0; i < scriptCount; i++) {
    JsonObject scriptObj = scriptsArray.add<JsonObject>();
    scriptObj["index"] = i;
    scriptObj["name"] = scriptActions[i].name;
    scriptObj["description"] = scriptActions[i].description;
    scriptObj["commands"] = scriptActions[i].commands;
    scriptObj["enabled"] = scriptActions[i].enabled;
  }
  
  File file = LittleFS.open(OFFLINE_CONFIG_FILE, "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
    DebugConsole::getInstance().log("Offline configuration saved to " + String(OFFLINE_CONFIG_FILE), "success");
  } else {
    DebugConsole::getInstance().log("Failed to save offline configuration", "error");
  }
}

void ServoController::loadOfflineConfiguration() {
  if (!LittleFS.exists(OFFLINE_CONFIG_FILE)) {
    DebugConsole::getInstance().log("No offline configuration file found", "info");
    return;
  }
  
  File file = LittleFS.open(OFFLINE_CONFIG_FILE, "r");
  if (!file) {
    DebugConsole::getInstance().log("Failed to open offline configuration file", "error");
    return;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    DebugConsole::getInstance().log("Failed to parse offline configuration file", "error");
    return;
  }
  
  JsonArray boardsArray = doc["boards"].as<JsonArray>();
  for (int b = 0; b < boardsArray.size() && b < detectedBoardCount; b++) {
    JsonObject boardObj = boardsArray[b];
    if (boardObj["address"] == boards[b].address) {
      strncpy(boards[b].name, boardObj["name"] | boards[b].name, sizeof(boards[b].name));
      boards[b].enabled = boardObj["enabled"] | boards[b].enabled;
      
      JsonArray servosArray = boardObj["servos"].as<JsonArray>();
      for (int s = 0; s < servosArray.size() && s < SERVOS_PER_BOARD; s++) {
        JsonObject servoObj = servosArray[s];
        servoConfigs[b][s].enabled = servoObj["enabled"];
        servoConfigs[b][s].center = servoObj["center"];
        servoConfigs[b][s].range = servoObj["range"];
        servoConfigs[b][s].initPosition = servoObj["initPosition"];
        servoConfigs[b][s].isPair = servoObj["isPair"];
        servoConfigs[b][s].pairBoard = servoObj["pairBoard"];
        servoConfigs[b][s].pairServo = servoObj["pairServo"];
        servoConfigs[b][s].isPairMaster = servoObj["isPairMaster"];
        strncpy(servoConfigs[b][s].name, servoObj["name"] | servoConfigs[b][s].name, sizeof(servoConfigs[b][s].name));
      }
    }
  }
  
  // Load scripts from offline configuration if they exist
  if (doc["scripts"].is<JsonArray>()) {
    JsonArray scriptsArray = doc["scripts"].as<JsonArray>();
    scriptCount = 0;
    for (int i = 0; i < scriptsArray.size() && i < MAX_SCRIPTS; i++) {
      JsonObject scriptObj = scriptsArray[i];
      strncpy(scriptActions[scriptCount].name, scriptObj["name"] | "", sizeof(scriptActions[scriptCount].name) - 1);
      scriptActions[scriptCount].name[sizeof(scriptActions[scriptCount].name) - 1] = '\0';
      
      strncpy(scriptActions[scriptCount].description, scriptObj["description"] | "", sizeof(scriptActions[scriptCount].description) - 1);
      scriptActions[scriptCount].description[sizeof(scriptActions[scriptCount].description) - 1] = '\0';
      
      strncpy(scriptActions[scriptCount].commands, scriptObj["commands"] | "", sizeof(scriptActions[scriptCount].commands) - 1);
      scriptActions[scriptCount].commands[sizeof(scriptActions[scriptCount].commands) - 1] = '\0';
      
      scriptActions[scriptCount].enabled = scriptObj["enabled"] | true;
      scriptCount++;
    }
  }
  
  DebugConsole::getInstance().log("Offline configuration loaded from " + String(OFFLINE_CONFIG_FILE), "success");
}