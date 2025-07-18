#pragma once

#include <Arduino.h>
#include <vector>
#include <ArduinoJson.h>

class DebugConsole {
private:
    static const int MAX_MESSAGES = 100;
    struct DebugMessage {
        String timestamp;
        String message;
        String type;
    };
    
    std::vector<DebugMessage> messages;
    
public:
    static DebugConsole& getInstance() {
        static DebugConsole instance;
        return instance;
    }
    
    void log(const String& message, const String& type = "info");
    void logf(const String& type, const char* format, ...);
    String getMessagesJson();
    void clear();
    
private:
    DebugConsole() {}
    String getCurrentTimestamp();
};