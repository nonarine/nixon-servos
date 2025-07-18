#include "ServoController.h"

bool ServoController::startSweep(int boardIndex, int servoIndex, float startPos, float endPos, unsigned long durationMs) {
  // Validate parameters
  if (boardIndex < 0 || boardIndex >= detectedBoardCount || 
      servoIndex < 0 || servoIndex >= SERVOS_PER_BOARD) {
    DebugConsole::getInstance().logf("error", "Invalid sweep parameters: board=%d, servo=%d", boardIndex, servoIndex);
    return false;
  }
  
  if (!servoConfigs[boardIndex][servoIndex].enabled) {
    DebugConsole::getInstance().logf("error", "Cannot sweep disabled servo %d:%d", boardIndex, servoIndex);
    return false;
  }
  
  if (durationMs == 0) {
    DebugConsole::getInstance().log("Sweep duration cannot be zero", "error");
    return false;
  }
  
  // Apply range limits
  float center = servoConfigs[boardIndex][servoIndex].center;
  float range = servoConfigs[boardIndex][servoIndex].range;
  float minPos = center - range;
  float maxPos = center + range;
  
  startPos = max(minPos, min(maxPos, startPos));
  endPos = max(minPos, min(maxPos, endPos));
  
  // Find existing sweep slot or create new one
  int sweepIndex = -1;
  for (int i = 0; i < MAX_BOARDS * SERVOS_PER_BOARD; i++) {
    if (sweepActions[i].active && 
        sweepActions[i].boardIndex == boardIndex && 
        sweepActions[i].servoIndex == servoIndex) {
      // Update existing sweep
      sweepIndex = i;
      break;
    }
  }
  
  if (sweepIndex == -1) {
    // Find empty slot
    for (int i = 0; i < MAX_BOARDS * SERVOS_PER_BOARD; i++) {
      if (!sweepActions[i].active) {
        sweepIndex = i;
        activeSweepCount++;
        break;
      }
    }
  }
  
  if (sweepIndex == -1) {
    DebugConsole::getInstance().log("No sweep slots available", "error");
    return false;
  }
  
  // Configure sweep
  sweepActions[sweepIndex].boardIndex = boardIndex;
  sweepActions[sweepIndex].servoIndex = servoIndex;
  sweepActions[sweepIndex].startPosition = startPos;
  sweepActions[sweepIndex].endPosition = endPos;
  sweepActions[sweepIndex].startTime = millis();
  sweepActions[sweepIndex].duration = durationMs;
  sweepActions[sweepIndex].active = true;
  
  // Set initial position
  setServoToConfiguredPosition(boardIndex, servoIndex, startPos);
  
  DebugConsole::getInstance().logf("success", "Started sweep: servo %d:%d from %.1f to %.1f over %lums", 
                                   boardIndex, servoIndex, startPos, endPos, durationMs);
  
  return true;
}

void ServoController::stopSweep(int boardIndex, int servoIndex) {
  for (int i = 0; i < MAX_BOARDS * SERVOS_PER_BOARD; i++) {
    if (sweepActions[i].active && 
        sweepActions[i].boardIndex == boardIndex && 
        sweepActions[i].servoIndex == servoIndex) {
      sweepActions[i].active = false;
      activeSweepCount--;
      DebugConsole::getInstance().logf("info", "Stopped sweep for servo %d:%d", boardIndex, servoIndex);
      return;
    }
  }
}

void ServoController::stopAllSweeps() {
  int stopped = 0;
  for (int i = 0; i < MAX_BOARDS * SERVOS_PER_BOARD; i++) {
    if (sweepActions[i].active) {
      sweepActions[i].active = false;
      stopped++;
    }
  }
  activeSweepCount = 0;
  DebugConsole::getInstance().logf("info", "Stopped %d active sweeps", stopped);
}

void ServoController::updateSweeps() {
  if (activeSweepCount == 0) return;
  
  unsigned long currentTime = millis();
  
  for (int i = 0; i < MAX_BOARDS * SERVOS_PER_BOARD; i++) {
    if (!sweepActions[i].active) continue;
    
    SweepAction& sweep = sweepActions[i];
    unsigned long elapsed = currentTime - sweep.startTime;
    
    if (elapsed >= sweep.duration) {
      // Sweep completed
      setServoToConfiguredPosition(sweep.boardIndex, sweep.servoIndex, sweep.endPosition);
      sweep.active = false;
      activeSweepCount--;
      DebugConsole::getInstance().logf("info", "Sweep completed: servo %d:%d", sweep.boardIndex, sweep.servoIndex);
    } else {
      // Calculate current position using linear interpolation
      float progress = (float)elapsed / (float)sweep.duration;
      float currentPosition = sweep.startPosition + (sweep.endPosition - sweep.startPosition) * progress;
      
      setServoToConfiguredPosition(sweep.boardIndex, sweep.servoIndex, currentPosition);
    }
  }
}