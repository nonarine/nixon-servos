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
  
  DebugConsole::getInstance().log("Executing script: " + name, "info");
  
  // Execute commands separated by semicolons with proper timing
  String commands = String(scriptActions[index].commands);
  DebugConsole::getInstance().log("Script commands: " + commands, "info");
  
  int startPos = 0;
  int endPos = commands.indexOf(';');
  unsigned long currentDelay = 0;
  int commandCount = 0;
  
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
      commandCount++;
      // Check if this is a sleep command
      String lowerCommand = command;
      lowerCommand.toLowerCase();
      if (lowerCommand.startsWith("sleep ")) {
        int sleepTime = command.substring(6).toInt();
        if (sleepTime > 0 && sleepTime <= 10000) {
          currentDelay += sleepTime;
          DebugConsole::getInstance().log("Sleep command: " + String(sleepTime) + "ms, total delay: " + String(currentDelay), "info");
        }
      } else {
        // Queue the command with accumulated delay
        DebugConsole::getInstance().log("Queuing command: " + command + " with delay: " + String(currentDelay), "info");
        queueCommand(command, currentDelay);
      }
    }
  }
  
  DebugConsole::getInstance().log("Script queued " + String(commandCount) + " commands", "success");
  
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