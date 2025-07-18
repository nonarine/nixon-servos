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