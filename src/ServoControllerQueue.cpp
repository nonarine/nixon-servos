#include "ServoController.h"

void ServoController::update() {
  unsigned long currentTime = millis();
  
  // Update sweep actions
  updateSweeps();
  
  // Update command sequences
  updateCommandSequence();
  
  // Process command queue
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
  
  DebugConsole::getInstance().log("Executing immediate command: " + command, "info");
  
  // Split command into parts
  int spaceIndex = cmd.indexOf(' ');
  String mainCommand = (spaceIndex > 0) ? cmd.substring(0, spaceIndex) : cmd;
  String args = (spaceIndex > 0) ? cmd.substring(spaceIndex + 1) : "";
  
  // Route to appropriate command handler (excluding sleep and script commands)
  if (mainCommand == "servo") {
    return executeServoCommand(args);
  } else if (mainCommand == "sweep") {
    return executeSweepCommand(args);
  } else if (mainCommand == "repeat") {
    return executeRepeatCommand(args);
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
    // Execute script by name
    if (args.length() == 0) {
      return "Error: script command requires a script name. Usage: script <name>";
    }
    if (executeScript(args)) {
      return "Success: Executed script '" + args + "'";
    } else {
      return "Error: Script '" + args + "' not found or disabled";
    }
  } else {
    return "Error: Unknown command '" + mainCommand + "'. Type 'help' for available commands.";
  }
}