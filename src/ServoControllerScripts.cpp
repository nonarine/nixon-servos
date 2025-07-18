#include "ServoController.h"

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
  // Prevent infinite recursion
  if (scriptRecursionDepth >= 5) {
    DebugConsole::getInstance().log("Script recursion limit exceeded (max 5): " + name, "error");
    return false;
  }
  
  // Find script by name
  int index = -1;
  for (int i = 0; i < scriptCount; i++) {
    if (String(scriptActions[i].name) == name) {
      index = i;
      break;
    }
  }
  
  if (index == -1) {
    DebugConsole::getInstance().log("Script not found: " + name, "error");
    return false;
  }
  
  if (!scriptActions[index].enabled) {
    DebugConsole::getInstance().log("Script disabled: " + name, "error");
    return false;
  }
  
  // Increment recursion depth
  scriptRecursionDepth++;
  
  DebugConsole::getInstance().log("Executing script: " + name, "info");
  
  // Parse commands separated by semicolons or newlines into sequence
  String commands = String(scriptActions[index].commands);
  DebugConsole::getInstance().log("Script commands: " + commands, "info");
  
  // Parse commands into array for sequence execution
  String* commandArray = new String[100]; // Max sequence size
  int commandCount = 0;
  int startPos = 0;
  
  // Helper function to find next command separator (semicolon or newline)
  auto findNextSeparator = [&commands](int fromPos) -> int {
    int semicolonPos = commands.indexOf(';', fromPos);
    int newlinePos = commands.indexOf('\n', fromPos);
    int crPos = commands.indexOf('\r', fromPos);
    
    // Find the earliest separator position
    int nextPos = -1;
    if (semicolonPos != -1) nextPos = semicolonPos;
    if (newlinePos != -1 && (nextPos == -1 || newlinePos < nextPos)) nextPos = newlinePos;
    if (crPos != -1 && (nextPos == -1 || crPos < nextPos)) nextPos = crPos;
    
    return nextPos;
  };
  
  int endPos = findNextSeparator(startPos);
  
  while (startPos < commands.length() && commandCount < 100) {
    String command;
    if (endPos == -1) {
      command = commands.substring(startPos);
      startPos = commands.length();
    } else {
      command = commands.substring(startPos, endPos);
      startPos = endPos + 1;
      endPos = findNextSeparator(startPos);
    }
    
    command.trim();
    if (command.length() > 0) {
      commandArray[commandCount] = command;
      commandCount++;
      DebugConsole::getInstance().log("Parsed command: " + command, "info");
    }
  }
  
  bool success = false;
  if (commandCount > 0) {
    // If we're already in a sequence (recursion), append commands to existing sequence
    if (commandSequence.active && scriptRecursionDepth > 1) {
      // Append commands to current sequence
      int remainingSlots = 100 - commandSequence.totalCount;
      int commandsToAdd = (commandCount > remainingSlots) ? remainingSlots : commandCount;
      
      for (int i = 0; i < commandsToAdd; i++) {
        commandSequence.commands[commandSequence.totalCount + i] = commandArray[i];
      }
      commandSequence.totalCount += commandsToAdd;
      
      DebugConsole::getInstance().log("Appended " + String(commandsToAdd) + " commands to existing sequence", "success");
      success = true;
    } else {
      // Start new command sequence for sequential execution
      success = startCommandSequence(commandArray, commandCount);
      if (success) {
        DebugConsole::getInstance().log("Started command sequence with " + String(commandCount) + " commands", "success");
      } else {
        DebugConsole::getInstance().log("Failed to start command sequence", "error");
      }
    }
  } else {
    DebugConsole::getInstance().log("No valid commands found in script", "error");
  }
  
  // Decrement recursion depth
  scriptRecursionDepth--;
  
  // Clean up
  delete[] commandArray;
  
  return success;
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