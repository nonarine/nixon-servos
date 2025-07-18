#include "ServoController.h"

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