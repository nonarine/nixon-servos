#include "ServoController.h"

ServoController::ServoController() {
  detectedBoardCount = 0;
  scriptCount = 0;
  activeSweepCount = 0;
  scriptRecursionDepth = 0;
  
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
  
  // Initialize sweep actions
  for (int i = 0; i < MAX_BOARDS * SERVOS_PER_BOARD; i++) {
    sweepActions[i].active = false;
  }
  
  // Initialize command sequence
  commandSequence.active = false;
  commandSequence.currentIndex = 0;
  commandSequence.totalCount = 0;
  commandSequence.waitUntil = 0;
  
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