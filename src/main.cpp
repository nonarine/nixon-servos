#include <Wire.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "wifi_credentials.h"
#include "ServoController.h"
#include "WebServer.h"

// Global objects
ServoController* servoController;
WebServerManager* webServer;

// Serial command buffer
String serialCommand = "";
bool serialCommandReady = false;




void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
    return;
  }
  Serial.println("LittleFS mounted successfully");
  
  // List files in LittleFS for debugging
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  Serial.println("Files in LittleFS:");
  while(file){
    Serial.print("  ");
    Serial.println(file.name());
    file = root.openNextFile();
  }
}

void initWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("nixon");
  Serial.println("WiFi Access Point started");
  Serial.print("AP SSID: nixon");
  Serial.println();
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}


void setup()
{
  Serial.begin(115200);
  Serial.println("Starting ESP32 Multi-Board Servo Controller...");
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize LittleFS
  initLittleFS();
  
  // Initialize WiFi
  initWiFi();
  
  // Create servo controller
  servoController = new ServoController();
  
  // Scan for PCA9685 boards
  servoController->scanForBoards();
  
  // Initialize detected boards
  servoController->initializeBoards();
  
  // Load configuration
  servoController->loadConfiguration();
  
  // Create and start web server
  webServer = new WebServerManager(servoController);
  webServer->begin();
  
  // Apply initial positions
  servoController->applyInitialPositions();
  
  Serial.println("Setup complete!");
  Serial.print("Visit http://");
  Serial.print(WiFi.softAPIP());
  Serial.println(" to configure servos");
  Serial.println("Type 'help' for serial command interface");
}

void processSerialInput() {
  // Read serial input character by character
  while (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n' || inChar == '\r') {
      // End of command
      if (serialCommand.length() > 0) {
        serialCommandReady = true;
      }
    } else if (inChar == 8 || inChar == 127) {
      // Backspace
      if (serialCommand.length() > 0) {
        serialCommand.remove(serialCommand.length() - 1);
        Serial.print("\b \b");
      }
    } else if (inChar >= 32 && inChar <= 126) {
      // Printable character
      serialCommand += inChar;
      Serial.print(inChar);
    }
  }
}

void executeSerialCommand() {
  if (serialCommandReady) {
    serialCommand.trim();
    
    if (serialCommand.length() > 0) {
      Serial.println();
      Serial.print("Executing: ");
      Serial.println(serialCommand);
      
      // Execute command through ServoController
      String result = servoController->executeCommand(serialCommand);
      
      // Print result
      Serial.print("Result: ");
      Serial.println(result);
    }
    
    // Reset command buffer
    serialCommand = "";
    serialCommandReady = false;
    
    // Print prompt
    Serial.print("servo> ");
  }
}

void loop()
{
  // Main loop - servos are now controlled via HTTP API
  // The web server handles requests asynchronously
  
  // Process queued commands with timers
  servoController->update();
  
  // Handle serial command input
  processSerialInput();
  executeSerialCommand();
  
  // Small delay to prevent watchdog issues
  delay(1);
}