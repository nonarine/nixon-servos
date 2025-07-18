#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <queue>
#include "DebugConsole.h"

#define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN  1000 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX  2000 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

#define USMIDPOINT ((USMAX+USMIN)/2)
#define USDEVIATION ((USMAX-USMIN)/2)

const int MAX_BOARDS = 8;        // Maximum number of PCA9685 boards
const int SERVOS_PER_BOARD = 16; // 16 servos per PCA9685 board
const int MAX_SCRIPTS = 20;      // Maximum number of script actions

// PCA9685 board structure
struct PCA9685Board {
  uint8_t address;
  bool detected;
  bool enabled;
  Adafruit_PWMServoDriver* driver;
  char name[32];
};

// Servo configuration structure
struct ServoConfig {
  bool enabled;           // Is this servo enabled?
  float center;           // Center position (0-100%)
  float range;            // Range around center (0-50%)
  float initPosition;     // Initial position (0-100%)
  bool isPair;            // Is this part of an inverse pair?
  int pairBoard;          // Board number of paired servo
  int pairServo;          // Servo number to pair with (-1 if not paired)
  bool isPairMaster;      // Is this the master servo in the pair?
  char name[32];          // Human-readable name
};

// Script action structure
struct ScriptAction {
  char name[32];          // Script name
  char description[64];   // Script description
  char commands[512];     // Script commands (separated by semicolons)
  bool enabled;           // Is this script active?
};

// Command queue item structure
struct QueuedCommand {
  String command;         // Command to execute
  unsigned long executeTime; // When to execute (millis())
};

struct CommandSequence {
  String commands[100];   // Array of commands to execute in sequence
  int currentIndex;       // Current command being executed
  int totalCount;         // Total number of commands
  unsigned long waitUntil; // Time to wait until before executing next command
  bool active;            // Whether this sequence is active
};

struct SweepAction {
  int boardIndex;
  int servoIndex;
  float startPosition;
  float endPosition;
  unsigned long startTime;
  unsigned long duration;
  bool active;
};

class ServoController {
private:
  PCA9685Board boards[MAX_BOARDS];
  ServoConfig servoConfigs[MAX_BOARDS][SERVOS_PER_BOARD];
  ScriptAction scriptActions[MAX_SCRIPTS];
  std::queue<QueuedCommand> commandQueue;
  SweepAction sweepActions[MAX_BOARDS * SERVOS_PER_BOARD];
  CommandSequence commandSequence;
  int detectedBoardCount;
  int scriptCount;
  int activeSweepCount;
  
  // Common I2C addresses for PCA9685 boards
  uint8_t commonAddresses[8] = {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47};
  
  // Configuration file paths
  const char* CONFIG_FILE = "/servo_config.json";
  const char* OFFLINE_CONFIG_FILE = "/offline_config.json";
  
public:
  ServoController();
  ~ServoController();
  
  // Hardware management
  void scanForBoards();
  void initializeBoards();
  void initializeServoConfigs();
  
  // Servo control
  void setServoByPercent(int boardIndex, int servonum, float pct);
  void setServoToConfiguredPosition(int boardIndex, int servonum, float position);
  void applyInitialPositions();
  
  // Sweep control
  bool startSweep(int boardIndex, int servoIndex, float startPos, float endPos, unsigned long durationMs);
  void stopSweep(int boardIndex, int servoIndex);
  void stopAllSweeps();
  void updateSweeps();
  
  // Configuration management
  void saveConfiguration();
  void loadConfiguration();
  void saveOfflineConfiguration();
  void loadOfflineConfiguration();
  
  // Getters
  int getDetectedBoardCount() const { return detectedBoardCount; }
  const PCA9685Board* getBoards() const { return boards; }
  const ServoConfig* getServoConfigs() const { return (const ServoConfig*)servoConfigs; }
  ServoConfig* getServoConfig(int boardIndex, int servoIndex);
  
  // JSON serialization
  String getSystemInfoJson();
  String getConfigurationJson();
  bool updateServoConfig(int boardIndex, int servoIndex, const String& field, const String& value);
  
  // Command interface
  String executeCommand(const String& command);
  
  // Script management
  bool addScript(const String& name, const String& description, const String& commands);
  bool updateScript(int index, const String& name, const String& description, const String& commands);
  bool deleteScript(int index);
  bool executeScript(int index);
  bool executeScript(const String& name);
  String getScriptsJson();
  ScriptAction* getScript(int index);
  int getScriptCount() const { return scriptCount; }
  
  // Timer and queue management
  void update();  // Call this regularly to process queued commands
  bool queueCommand(const String& command, unsigned long delayMs = 0);
  void clearQueue();
  
  // Command sequence management
  bool startCommandSequence(String* commands, int count);
  void updateCommandSequence();
  
private:
  // Command parsing helpers
  String parseCommand(const String& command);
  String executeServoCommand(const String& args);
  String executeSystemCommand(const String& args);
  String executeConfigCommand(const String& args);
  String executePairCommand(const String& args);
  String executeSweepCommand(const String& args);
  String executeRepeatCommand(const String& args);
  String executeHelpCommand();
  
  // Queue management helpers
  String executeCommandImmediate(const String& command);
};