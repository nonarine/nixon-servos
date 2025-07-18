#include "ServoController.h"

ServoConfig* ServoController::getServoConfig(int boardIndex, int servoIndex) {
  if (boardIndex >= 0 && boardIndex < MAX_BOARDS && 
      servoIndex >= 0 && servoIndex < SERVOS_PER_BOARD) {
    return &servoConfigs[boardIndex][servoIndex];
  }
  return nullptr;
}

String ServoController::getSystemInfoJson() {
  JsonDocument doc;
  doc["success"] = true;
  doc["boardCount"] = detectedBoardCount;
  doc["servosPerBoard"] = SERVOS_PER_BOARD;
  
  JsonArray boardsArray = doc["boards"].to<JsonArray>();
  for (int b = 0; b < detectedBoardCount; b++) {
    JsonObject boardObj = boardsArray.add<JsonObject>();
    boardObj["index"] = b;
    boardObj["address"] = boards[b].address;
    boardObj["name"] = boards[b].name;
    boardObj["enabled"] = boards[b].enabled;
  }
  
  String response;
  serializeJson(doc, response);
  return response;
}

String ServoController::getConfigurationJson() {
  JsonDocument doc;
  doc["success"] = true;
  
  JsonArray boardsArray = doc["boards"].to<JsonArray>();
  for (int b = 0; b < detectedBoardCount; b++) {
    JsonObject boardObj = boardsArray.add<JsonObject>();
    boardObj["index"] = b;
    boardObj["address"] = boards[b].address;
    boardObj["name"] = boards[b].name;
    boardObj["enabled"] = boards[b].enabled;
    
    JsonArray servosArray = boardObj["servos"].to<JsonArray>();
    for (int s = 0; s < SERVOS_PER_BOARD; s++) {
      JsonObject servoObj = servosArray.add<JsonObject>();
      servoObj["index"] = s;
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
  
  String response;
  serializeJson(doc, response);
  return response;
}

bool ServoController::updateServoConfig(int boardIndex, int servoIndex, const String& field, const String& value) {
  DebugConsole::getInstance().logf("info", "updateServoConfig: board=%d, servo=%d, field=%s, value=%s", boardIndex, servoIndex, field.c_str(), value.c_str());
  
  if (boardIndex < 0 || boardIndex >= detectedBoardCount || 
      servoIndex < 0 || servoIndex >= SERVOS_PER_BOARD) {
    DebugConsole::getInstance().logf("error", "Invalid board/servo index: board=%d, servo=%d", boardIndex, servoIndex);
    return false;
  }
  
  ServoConfig* config = &servoConfigs[boardIndex][servoIndex];
  
  if (field == "enabled") {
    bool newEnabled = (value == "true");
    DebugConsole::getInstance().logf("info", "Setting servo %d:%d enabled from %s to %s", boardIndex, servoIndex, config->enabled ? "true" : "false", newEnabled ? "true" : "false");
    config->enabled = newEnabled;
  } else if (field == "center") {
    config->center = value.toFloat();
  } else if (field == "range") {
    config->range = value.toFloat();
  } else if (field == "initPosition") {
    config->initPosition = value.toFloat();
  } else if (field == "isPair") {
    config->isPair = (value == "true");
  } else if (field == "pairBoard") {
    config->pairBoard = value.toInt();
  } else if (field == "pairServo") {
    config->pairServo = value.toInt();
  } else if (field == "isPairMaster") {
    config->isPairMaster = (value == "true");
  } else if (field == "name") {
    strncpy(config->name, value.c_str(), sizeof(config->name) - 1);
    config->name[sizeof(config->name) - 1] = '\0';
  } else {
    return false;
  }
  
  return true;
}