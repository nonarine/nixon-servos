#include <Wire.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "wifi_credentials.h"
#include "ServoController.h"
#include "WebServer.h"

// Global objects
ServoController* servoController;
WebServerManager* webServer;




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
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected to WiFi. IP: ");
  Serial.println(WiFi.localIP());
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
  Serial.print(WiFi.localIP());
  Serial.println(" to configure servos");
}

void loop()
{
  // Main loop - servos are now controlled via HTTP API
  // The web server handles requests asynchronously
  // No active servo control needed in the main loop
  
  // Small delay to prevent watchdog issues
  delay(10);
}