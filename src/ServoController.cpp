#include "ServoController.h"

ServoController::ServoController() {
  detectedBoardCount = 0;
  scriptCount = 0;
  
  // Initialize all board pointers to nullptr
  for (int i = 0; i < MAX_BOARDS; i++) {
    boards[i].detected = false;
    boards[i].enabled = false;
    boards[i].driver = nullptr;
    boards[i].address = 0;
    strcpy(boards[i].name, "");
  }
  
  // Initialize script actions
  for (int i = 0; i < MAX_SCRIPTS; i++) {
    strcpy(scriptActions[i].name, "");
    strcpy(scriptActions[i].description, "");
    strcpy(scriptActions[i].commands, "");
    scriptActions[i].enabled = false;
  }
  
  initializeServoConfigs();
}

ServoController::~ServoController() {
  // Clean up dynamically allocated drivers
  for (int i = 0; i < MAX_BOARDS; i++) {
    if (boards[i].driver) {
      delete boards[i].driver;
      boards[i].driver = nullptr;
    }
  }
}

void ServoController::setServoByPercent(int boardIndex, int servonum, float pct) {
  if (boardIndex < 0 || boardIndex >= MAX_BOARDS) return;
  if (servonum < 0 || servonum >= SERVOS_PER_BOARD) return;
  if (!boards[boardIndex].detected || !boards[boardIndex].enabled) return;
  
  boards[boardIndex].driver->writeMicroseconds(servonum,
      uint16_t((((100.0 - pct) * USMIN) / 100.0) + ((pct * USMAX) / 100.0))
  );
}

void ServoController::setServoToConfiguredPosition(int boardIndex, int servonum, float position) {
  if (boardIndex < 0 || boardIndex >= MAX_BOARDS) return;
  if (servonum < 0 || servonum >= SERVOS_PER_BOARD) return;
  if (!servoConfigs[boardIndex][servonum].enabled) return;
  
  // Apply range limits
  float center = servoConfigs[boardIndex][servonum].center;
  float range = servoConfigs[boardIndex][servonum].range;
  float minPos = center - range;
  float maxPos = center + range;
  
  // Clamp position to configured range
  position = max(minPos, min(maxPos, position));
  
  setServoByPercent(boardIndex, servonum, position);
  
  // Handle inverse pair
  if (servoConfigs[boardIndex][servonum].isPair && servoConfigs[boardIndex][servonum].isPairMaster) {
    int pairBoard = servoConfigs[boardIndex][servonum].pairBoard;
    int pairServo = servoConfigs[boardIndex][servonum].pairServo;
    
    if (pairBoard >= 0 && pairBoard < detectedBoardCount && 
        pairServo >= 0 && pairServo < SERVOS_PER_BOARD && 
        servoConfigs[pairBoard][pairServo].enabled) {
      
      float pairCenter = servoConfigs[pairBoard][pairServo].center;
      float pairRange = servoConfigs[pairBoard][pairServo].range;
      float pairPosition = pairCenter - (position - center); // Inverse relationship
      
      // Apply pair servo range limits
      float pairMinPos = pairCenter - pairRange;
      float pairMaxPos = pairCenter + pairRange;
      pairPosition = max(pairMinPos, min(pairMaxPos, pairPosition));
      
      setServoByPercent(pairBoard, pairServo, pairPosition);
      
      DebugConsole::getInstance().logf("success", "Pairing: Board %d Servo %d (%.1f%%) -> Board %d Servo %d (%.1f%%)", 
                    boardIndex, servonum, position, pairBoard, pairServo, pairPosition);
    } else {
      DebugConsole::getInstance().logf("error", "Pairing failed: Board %d Servo %d - Invalid pair target (Board %d, Servo %d)", 
                    boardIndex, servonum, pairBoard, pairServo);
    }
  }
}

void ServoController::scanForBoards() {
  DebugConsole::getInstance().log("Scanning for PCA9685 boards...", "info");
  
  // Clean up existing drivers first
  for (int i = 0; i < MAX_BOARDS; i++) {
    if (boards[i].driver) {
      delete boards[i].driver;
      boards[i].driver = nullptr;
    }
    boards[i].detected = false;
    boards[i].enabled = false;
  }
  
  detectedBoardCount = 0;
  
  // Scan common I2C addresses
  for (int i = 0; i < 8 && detectedBoardCount < MAX_BOARDS; i++) {
    Wire.beginTransmission(commonAddresses[i]);
    if (Wire.endTransmission() == 0) {
      // Device found
      boards[detectedBoardCount].address = commonAddresses[i];
      boards[detectedBoardCount].detected = true;
      boards[detectedBoardCount].enabled = true;
      boards[detectedBoardCount].driver = new Adafruit_PWMServoDriver(commonAddresses[i]);
      snprintf(boards[detectedBoardCount].name, sizeof(boards[detectedBoardCount].name), 
               "PCA9685 @0x%02X", commonAddresses[i]);
      
      DebugConsole::getInstance().logf("success", "Found PCA9685 at address 0x%02X", commonAddresses[i]);
      detectedBoardCount++;
    }
  }
  
  DebugConsole::getInstance().logf("success", "Found %d PCA9685 boards", detectedBoardCount);
}

void ServoController::initializeBoards() {
  for (int i = 0; i < detectedBoardCount; i++) {
    if (boards[i].driver) {
      boards[i].driver->begin();
      boards[i].driver->setOscillatorFrequency(27000000);
      boards[i].driver->setPWMFreq(SERVO_FREQ);
      delay(10);
    }
  }
}

void ServoController::initializeServoConfigs() {
  for (int b = 0; b < MAX_BOARDS; b++) {
    for (int s = 0; s < SERVOS_PER_BOARD; s++) {
      servoConfigs[b][s].enabled = false;
      servoConfigs[b][s].center = 50.0;
      servoConfigs[b][s].range = 50.0;
      servoConfigs[b][s].initPosition = 50.0;
      servoConfigs[b][s].isPair = false;
      servoConfigs[b][s].pairBoard = -1;
      servoConfigs[b][s].pairServo = -1;
      servoConfigs[b][s].isPairMaster = false;
      snprintf(servoConfigs[b][s].name, sizeof(servoConfigs[b][s].name), "Board %d Servo %d", b, s);
    }
  }
}

void ServoController::applyInitialPositions() {
  for (int b = 0; b < detectedBoardCount; b++) {
    for (int s = 0; s < SERVOS_PER_BOARD; s++) {
      if (servoConfigs[b][s].enabled) {
        setServoToConfiguredPosition(b, s, servoConfigs[b][s].initPosition);
      }
    }
  }
}

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
  
  DebugConsole::getInstance().log("Offline configuration loaded from " + String(OFFLINE_CONFIG_FILE), "success");
}

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
  if (boardIndex < 0 || boardIndex >= detectedBoardCount || 
      servoIndex < 0 || servoIndex >= SERVOS_PER_BOARD) {
    return false;
  }
  
  ServoConfig* config = &servoConfigs[boardIndex][servoIndex];
  
  if (field == "enabled") {
    config->enabled = (value == "true");
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

// Command Interface Implementation
String ServoController::executeCommand(const String& command) {
  String cmd = command;
  cmd.trim();
  cmd.toLowerCase();
  
  if (cmd.length() == 0) {
    return "Error: Empty command";
  }
  
  // Split command into parts
  int spaceIndex = cmd.indexOf(' ');
  String mainCommand = (spaceIndex > 0) ? cmd.substring(0, spaceIndex) : cmd;
  String args = (spaceIndex > 0) ? cmd.substring(spaceIndex + 1) : "";
  
  // Route to appropriate command handler
  if (mainCommand == "servo") {
    return executeServoCommand(args);
  } else if (mainCommand == "system") {
    return executeSystemCommand(args);
  } else if (mainCommand == "config") {
    return executeConfigCommand(args);
  } else if (mainCommand == "pair") {
    return executePairCommand(args);
  } else if (mainCommand == "script") {
    if (args.length() == 0) {
      return "Error: script command requires a script name. Usage: script <name>";
    }
    if (executeScript(args)) {
      return "Success: Executed script '" + args + "'";
    } else {
      return "Error: Script '" + args + "' not found or disabled";
    }
  } else if (mainCommand == "sleep") {
    if (args.length() == 0) {
      return "Error: sleep command requires a time in milliseconds. Usage: sleep <milliseconds>";
    }
    int sleepTime = args.toInt();
    if (sleepTime <= 0) {
      return "Error: Sleep time must be a positive number";
    }
    if (sleepTime > 10000) {
      return "Error: Sleep time cannot exceed 10000ms (10 seconds)";
    }
    // Queue the sleep command for later execution
    queueCommand("sleep " + String(sleepTime), sleepTime);
    return "Success: Sleep for " + String(sleepTime) + "ms scheduled";
  } else if (mainCommand == "help") {
    return executeHelpCommand();
  } else {
    return "Error: Unknown command '" + mainCommand + "'. Type 'help' for available commands.";
  }
}

String ServoController::executeServoCommand(const String& args) {
  if (args.length() == 0) {
    return "Error: servo command requires arguments. Usage: servo <board> <servo> <position>";
  }
  
  // Parse arguments: board servo position
  int firstSpace = args.indexOf(' ');
  int secondSpace = args.indexOf(' ', firstSpace + 1);
  
  if (firstSpace == -1 || secondSpace == -1) {
    return "Error: servo command requires 3 arguments: board servo position";
  }
  
  int boardIndex = args.substring(0, firstSpace).toInt();
  int servoIndex = args.substring(firstSpace + 1, secondSpace).toInt();
  float position = args.substring(secondSpace + 1).toFloat();
  
  // Validate arguments
  if (boardIndex < 0 || boardIndex >= detectedBoardCount) {
    return "Error: Invalid board index " + String(boardIndex) + ". Available boards: 0-" + String(detectedBoardCount - 1);
  }
  
  if (servoIndex < 0 || servoIndex >= SERVOS_PER_BOARD) {
    return "Error: Invalid servo index " + String(servoIndex) + ". Valid range: 0-15";
  }
  
  if (position < 0.0 || position > 100.0) {
    return "Error: Position must be between 0.0 and 100.0";
  }
  
  // Execute servo movement
  setServoToConfiguredPosition(boardIndex, servoIndex, position);
  
  return "Success: Moved servo " + String(boardIndex) + ":" + String(servoIndex) + " to " + String(position) + "%";
}

String ServoController::executeSystemCommand(const String& args) {
  if (args.length() == 0) {
    return "Error: system command requires arguments. Usage: system <info|init|save|load>";
  }
  
  if (args == "info") {
    return "System Info - Boards: " + String(detectedBoardCount) + ", Total Servos: " + String(detectedBoardCount * SERVOS_PER_BOARD);
  } else if (args == "init") {
    applyInitialPositions();
    return "Success: Applied initial positions to all enabled servos";
  } else if (args == "save") {
    saveConfiguration();
    return "Success: Configuration saved";
  } else if (args == "load") {
    loadConfiguration();
    return "Success: Configuration loaded";
  } else {
    return "Error: Unknown system command '" + args + "'. Available: info, init, save, load";
  }
}

String ServoController::executeConfigCommand(const String& args) {
  if (args.length() == 0) {
    return "Error: config command requires arguments. Usage: config <board> <servo> <field> <value>";
  }
  
  // Parse arguments: board servo field value
  int spaces[3];
  int spaceCount = 0;
  int searchFrom = 0;
  
  for (int i = 0; i < 3; i++) {
    int spaceIndex = args.indexOf(' ', searchFrom);
    if (spaceIndex == -1) break;
    spaces[spaceCount++] = spaceIndex;
    searchFrom = spaceIndex + 1;
  }
  
  if (spaceCount < 3) {
    return "Error: config command requires 4 arguments: board servo field value";
  }
  
  int boardIndex = args.substring(0, spaces[0]).toInt();
  int servoIndex = args.substring(spaces[0] + 1, spaces[1]).toInt();
  String field = args.substring(spaces[1] + 1, spaces[2]);
  String value = args.substring(spaces[2] + 1);
  
  // Validate and execute
  if (boardIndex < 0 || boardIndex >= detectedBoardCount) {
    return "Error: Invalid board index " + String(boardIndex);
  }
  
  if (servoIndex < 0 || servoIndex >= SERVOS_PER_BOARD) {
    return "Error: Invalid servo index " + String(servoIndex);
  }
  
  if (updateServoConfig(boardIndex, servoIndex, field, value)) {
    return "Success: Updated " + field + " to " + value + " for servo " + String(boardIndex) + ":" + String(servoIndex);
  } else {
    return "Error: Failed to update " + field + ". Valid fields: enabled, center, range, initPosition, isPair, pairBoard, pairServo, isPairMaster, name";
  }
}

String ServoController::executePairCommand(const String& args) {
  if (args.length() == 0) {
    return "Error: pair command requires arguments. Usage: pair <board1> <servo1> <board2> <servo2>";
  }
  
  // Parse arguments: board1 servo1 board2 servo2
  int spaces[3];
  int spaceCount = 0;
  int searchFrom = 0;
  
  for (int i = 0; i < 3; i++) {
    int spaceIndex = args.indexOf(' ', searchFrom);
    if (spaceIndex == -1) break;
    spaces[spaceCount++] = spaceIndex;
    searchFrom = spaceIndex + 1;
  }
  
  if (spaceCount < 3) {
    return "Error: pair command requires 4 arguments: board1 servo1 board2 servo2";
  }
  
  int board1 = args.substring(0, spaces[0]).toInt();
  int servo1 = args.substring(spaces[0] + 1, spaces[1]).toInt();
  int board2 = args.substring(spaces[1] + 1, spaces[2]).toInt();
  int servo2 = args.substring(spaces[2] + 1).toInt();
  
  // Validate arguments
  if (board1 < 0 || board1 >= detectedBoardCount || board2 < 0 || board2 >= detectedBoardCount) {
    return "Error: Invalid board index";
  }
  
  if (servo1 < 0 || servo1 >= SERVOS_PER_BOARD || servo2 < 0 || servo2 >= SERVOS_PER_BOARD) {
    return "Error: Invalid servo index";
  }
  
  // Set up pairing - servo1 is master, servo2 is slave
  ServoConfig* master = &servoConfigs[board1][servo1];
  ServoConfig* slave = &servoConfigs[board2][servo2];
  
  master->isPair = true;
  master->isPairMaster = true;
  master->pairBoard = board2;
  master->pairServo = servo2;
  
  slave->isPair = true;
  slave->isPairMaster = false;
  slave->pairBoard = -1;
  slave->pairServo = -1;
  
  return "Success: Paired servo " + String(board1) + ":" + String(servo1) + " (master) with " + String(board2) + ":" + String(servo2) + " (slave)";
}

String ServoController::executeHelpCommand() {
  return "Available commands:\n"
         "servo <board> <servo> <position> - Move servo to position (0-100%)\n"
         "system info - Show system information\n"
         "system init - Apply initial positions to all servos\n"
         "system save - Save current configuration\n"
         "system load - Load saved configuration\n"
         "config <board> <servo> <field> <value> - Update servo configuration\n"
         "pair <board1> <servo1> <board2> <servo2> - Pair two servos (first is master)\n"
         "script <name> - Execute a saved script\n"
         "sleep <milliseconds> - Wait for specified time (max 10000ms)\n"
         "help - Show this help message";
}

// Script Management Functions
bool ServoController::addScript(const String& name, const String& description, const String& commands) {
  if (scriptCount >= MAX_SCRIPTS) {
    return false; // No more space
  }
  
  // Check if script name already exists
  for (int i = 0; i < scriptCount; i++) {
    if (String(scriptActions[i].name) == name) {
      return false; // Name already exists
    }
  }
  
  // Add new script
  strncpy(scriptActions[scriptCount].name, name.c_str(), sizeof(scriptActions[scriptCount].name) - 1);
  scriptActions[scriptCount].name[sizeof(scriptActions[scriptCount].name) - 1] = '\0';
  
  strncpy(scriptActions[scriptCount].description, description.c_str(), sizeof(scriptActions[scriptCount].description) - 1);
  scriptActions[scriptCount].description[sizeof(scriptActions[scriptCount].description) - 1] = '\0';
  
  strncpy(scriptActions[scriptCount].commands, commands.c_str(), sizeof(scriptActions[scriptCount].commands) - 1);
  scriptActions[scriptCount].commands[sizeof(scriptActions[scriptCount].commands) - 1] = '\0';
  
  scriptActions[scriptCount].enabled = true;
  scriptCount++;
  
  return true;
}

bool ServoController::updateScript(int index, const String& name, const String& description, const String& commands) {
  if (index < 0 || index >= scriptCount) {
    return false;
  }
  
  // Check if new name conflicts with existing scripts (except current one)
  for (int i = 0; i < scriptCount; i++) {
    if (i != index && String(scriptActions[i].name) == name) {
      return false; // Name already exists
    }
  }
  
  // Update script
  strncpy(scriptActions[index].name, name.c_str(), sizeof(scriptActions[index].name) - 1);
  scriptActions[index].name[sizeof(scriptActions[index].name) - 1] = '\0';
  
  strncpy(scriptActions[index].description, description.c_str(), sizeof(scriptActions[index].description) - 1);
  scriptActions[index].description[sizeof(scriptActions[index].description) - 1] = '\0';
  
  strncpy(scriptActions[index].commands, commands.c_str(), sizeof(scriptActions[index].commands) - 1);
  scriptActions[index].commands[sizeof(scriptActions[index].commands) - 1] = '\0';
  
  return true;
}

bool ServoController::deleteScript(int index) {
  if (index < 0 || index >= scriptCount) {
    return false;
  }
  
  // Shift scripts down
  for (int i = index; i < scriptCount - 1; i++) {
    scriptActions[i] = scriptActions[i + 1];
  }
  
  scriptCount--;
  return true;
}

bool ServoController::executeScript(int index) {
  if (index < 0 || index >= scriptCount || !scriptActions[index].enabled) {
    return false;
  }
  
  return executeScript(String(scriptActions[index].name));
}

bool ServoController::executeScript(const String& name) {
  // Find script by name
  int index = -1;
  for (int i = 0; i < scriptCount; i++) {
    if (String(scriptActions[i].name) == name) {
      index = i;
      break;
    }
  }
  
  if (index == -1 || !scriptActions[index].enabled) {
    return false;
  }
  
  // Execute commands separated by semicolons with proper timing
  String commands = String(scriptActions[index].commands);
  int startPos = 0;
  int endPos = commands.indexOf(';');
  unsigned long currentDelay = 0;
  
  while (startPos < commands.length()) {
    String command;
    if (endPos == -1) {
      command = commands.substring(startPos);
      startPos = commands.length();
    } else {
      command = commands.substring(startPos, endPos);
      startPos = endPos + 1;
      endPos = commands.indexOf(';', startPos);
    }
    
    command.trim();
    if (command.length() > 0) {
      // Check if this is a sleep command
      if (command.toLowerCase().startsWith("sleep ")) {
        int sleepTime = command.substring(6).toInt();
        if (sleepTime > 0 && sleepTime <= 10000) {
          currentDelay += sleepTime;
        }
      } else {
        // Queue the command with accumulated delay
        queueCommand(command, currentDelay);
      }
    }
  }
  
  return true;
}

String ServoController::getScriptsJson() {
  JsonDocument doc;
  doc["success"] = true;
  doc["count"] = scriptCount;
  
  JsonArray scripts = doc["scripts"].to<JsonArray>();
  
  for (int i = 0; i < scriptCount; i++) {
    JsonObject script = scripts.add<JsonObject>();
    script["index"] = i;
    script["name"] = scriptActions[i].name;
    script["description"] = scriptActions[i].description;
    script["commands"] = scriptActions[i].commands;
    script["enabled"] = scriptActions[i].enabled;
  }
  
  String output;
  serializeJson(doc, output);
  return output;
}

ScriptAction* ServoController::getScript(int index) {
  if (index < 0 || index >= scriptCount) {
    return nullptr;
  }
  return &scriptActions[index];
}

// Timer and queue management implementation
void ServoController::update() {
  unsigned long currentTime = millis();
  
  while (!commandQueue.empty()) {
    QueuedCommand& nextCommand = commandQueue.front();
    
    if (currentTime >= nextCommand.executeTime) {
      // Execute the command
      executeCommandImmediate(nextCommand.command);
      commandQueue.pop();
    } else {
      // Commands are queued in order, so if this one isn't ready, none after it are
      break;
    }
  }
}

bool ServoController::queueCommand(const String& command, unsigned long delayMs) {
  QueuedCommand queuedCmd;
  queuedCmd.command = command;
  queuedCmd.executeTime = millis() + delayMs;
  
  commandQueue.push(queuedCmd);
  return true;
}

void ServoController::clearQueue() {
  while (!commandQueue.empty()) {
    commandQueue.pop();
  }
}

String ServoController::executeCommandImmediate(const String& command) {
  String cmd = command;
  cmd.trim();
  cmd.toLowerCase();
  
  if (cmd.length() == 0) {
    return "Error: Empty command";
  }
  
  // Split command into parts
  int spaceIndex = cmd.indexOf(' ');
  String mainCommand = (spaceIndex > 0) ? cmd.substring(0, spaceIndex) : cmd;
  String args = (spaceIndex > 0) ? cmd.substring(spaceIndex + 1) : "";
  
  // Route to appropriate command handler (excluding sleep and script commands)
  if (mainCommand == "servo") {
    return executeServoCommand(args);
  } else if (mainCommand == "system") {
    return executeSystemCommand(args);
  } else if (mainCommand == "config") {
    return executeConfigCommand(args);
  } else if (mainCommand == "pair") {
    return executePairCommand(args);
  } else if (mainCommand == "help") {
    return executeHelpCommand();
  } else if (mainCommand == "sleep") {
    // Sleep commands should not be executed immediately
    return "Error: Sleep command cannot be executed immediately";
  } else if (mainCommand == "script") {
    // Script commands should not be executed immediately to avoid recursion
    return "Error: Script command cannot be executed immediately";
  } else {
    return "Error: Unknown command '" + mainCommand + "'. Type 'help' for available commands.";
  }
}