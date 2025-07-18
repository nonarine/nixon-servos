#include "ServoController.h"

void ServoController::setServoByPercent(int boardIndex, int servonum, float pct) {
  if (boardIndex < 0 || boardIndex >= MAX_BOARDS) return;
  if (servonum < 0 || servonum >= SERVOS_PER_BOARD) return;
  if (!boards[boardIndex].detected || !boards[boardIndex].enabled) return;
  
  boards[boardIndex].driver->writeMicroseconds(servonum,
      uint16_t((((100.0 - pct) * USMIN) / 100.0) + ((pct * USMAX) / 100.0))
  );
}

void ServoController::setServoToConfiguredPosition(int boardIndex, int servonum, float position) {
  DebugConsole::getInstance().logf("info", "setServoToConfiguredPosition: board=%d, servo=%d, position=%.1f", boardIndex, servonum, position);
  
  if (boardIndex < 0 || boardIndex >= MAX_BOARDS) {
    DebugConsole::getInstance().logf("error", "Invalid board index: %d", boardIndex);
    return;
  }
  if (servonum < 0 || servonum >= SERVOS_PER_BOARD) {
    DebugConsole::getInstance().logf("error", "Invalid servo index: %d", servonum);
    return;
  }
  if (!servoConfigs[boardIndex][servonum].enabled) {
    DebugConsole::getInstance().logf("error", "Servo %d:%d not enabled", boardIndex, servonum);
    return;
  }
  
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