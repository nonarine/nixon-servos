#include "ServoController.h"

bool ServoController::startCommandSequence(String* commands, int count) {
  if (count <= 0 || count > 100) {
    DebugConsole::getInstance().logf("error", "Invalid sequence count: %d", count);
    return false;
  }
  
  // Stop any existing sequence
  commandSequence.active = false;
  
  // Copy commands to sequence
  for (int i = 0; i < count; i++) {
    commandSequence.commands[i] = commands[i];
  }
  
  commandSequence.currentIndex = 0;
  commandSequence.totalCount = count;
  commandSequence.waitUntil = millis();
  commandSequence.active = true;
  
  DebugConsole::getInstance().logf("info", "Started command sequence with %d commands", count);
  
  return true;
}

void ServoController::updateCommandSequence() {
  if (!commandSequence.active) return;
  
  unsigned long currentTime = millis();
  
  // Check if we're still waiting
  if (currentTime < commandSequence.waitUntil) {
    return;
  }
  
  // Check if sequence is complete
  if (commandSequence.currentIndex >= commandSequence.totalCount) {
    commandSequence.active = false;
    DebugConsole::getInstance().log("Command sequence completed", "success");
    return;
  }
  
  // Get the current command
  String currentCommand = commandSequence.commands[commandSequence.currentIndex];
  currentCommand.trim();
  
  if (currentCommand.length() == 0) {
    // Skip empty commands
    commandSequence.currentIndex++;
    return;
  }
  
  DebugConsole::getInstance().logf("info", "Executing sequence command %d: %s", 
                                   commandSequence.currentIndex + 1, currentCommand.c_str());
  
  // Execute the command
  String result = executeCommandImmediate(currentCommand);
  DebugConsole::getInstance().logf("info", "Sequence command result: %s", result.c_str());
  
  // Determine wait time for next command
  unsigned long waitTime = 0;
  String lowerCommand = currentCommand;
  lowerCommand.toLowerCase();
  
  if (lowerCommand.startsWith("sweep ")) {
    // For sweep commands, extract the duration and wait for completion
    int lastSpaceIndex = currentCommand.lastIndexOf(' ');
    if (lastSpaceIndex != -1) {
      String durationStr = currentCommand.substring(lastSpaceIndex + 1);
      unsigned long sweepDuration = durationStr.toInt();
      if (sweepDuration > 0) {
        waitTime = sweepDuration + 100; // Add 100ms buffer
      }
    }
  } else if (lowerCommand.startsWith("sleep ")) {
    // For sleep commands, get the sleep duration
    int spaceIndex = currentCommand.indexOf(' ');
    if (spaceIndex != -1) {
      String durationStr = currentCommand.substring(spaceIndex + 1);
      unsigned long sleepDuration = durationStr.toInt();
      if (sleepDuration > 0 && sleepDuration <= 10000) {
        waitTime = sleepDuration;
      }
    }
  }
  // For other commands (servo, config, etc.), execute immediately (waitTime = 0)
  
  // Set next execution time
  commandSequence.waitUntil = currentTime + waitTime;
  commandSequence.currentIndex++;
  
  if (waitTime > 0) {
    DebugConsole::getInstance().logf("info", "Waiting %lu ms before next command", waitTime);
  }
}