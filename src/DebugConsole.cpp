#include "DebugConsole.h"
#include <WiFi.h>

String DebugConsole::getCurrentTimestamp() {
    // Get time since boot in milliseconds
    unsigned long ms = millis();
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    ms %= 1000;
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    
    return String(hours) + ":" + 
           (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
           (seconds < 10 ? "0" : "") + String(seconds) + "." + 
           (ms < 100 ? "0" : "") + (ms < 10 ? "0" : "") + String(ms);
}

void DebugConsole::log(const String& message, const String& type) {
    DebugMessage msg;
    msg.timestamp = getCurrentTimestamp();
    msg.message = message;
    msg.type = type;
    
    messages.push_back(msg);
    
    // Keep only the last MAX_MESSAGES
    if (messages.size() > MAX_MESSAGES) {
        messages.erase(messages.begin());
    }
}

void DebugConsole::logf(const String& type, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    va_end(args);
    
    log(String(buffer), type);
}

String DebugConsole::getMessagesJson() {
    JsonDocument doc;
    JsonArray msgArray = doc["messages"].to<JsonArray>();
    
    for (const auto& msg : messages) {
        JsonObject msgObj = msgArray.add<JsonObject>();
        msgObj["timestamp"] = msg.timestamp;
        msgObj["message"] = msg.message;
        msgObj["type"] = msg.type;
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}

void DebugConsole::clear() {
    messages.clear();
}