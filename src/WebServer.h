#pragma once

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "ServoController.h"
#include "DebugConsole.h"

class WebServerManager {
private:
  AsyncWebServer* server;
  ServoController* servoController;
  
public:
  WebServerManager(ServoController* controller);
  ~WebServerManager();
  
  void begin();
  void setupRoutes();
  void handleNotFound(AsyncWebServerRequest *request);
  
  // API handlers
  void handleGetInfo(AsyncWebServerRequest *request);
  void handleGetConfig(AsyncWebServerRequest *request);
  void handlePostConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  void handleTestServo(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  void handleInitServos(AsyncWebServerRequest *request);
  void handleSaveOffline(AsyncWebServerRequest *request);
  void handleLoadOffline(AsyncWebServerRequest *request);
  void handleGetDebug(AsyncWebServerRequest *request);
  void handleClearDebug(AsyncWebServerRequest *request);
  void handleCommand(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  
  // Script management handlers
  void handleGetScripts(AsyncWebServerRequest *request);
  void handlePostScript(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  void handlePutScript(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  void handleDeleteScript(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  void handleExecuteScript(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
};