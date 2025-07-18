# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a PlatformIO-based ESP32 project that controls servo motors using multiple PCA9685 I2C PWM controllers. The system supports up to 8 PCA9685 boards (128 servos total) with:
- **Advanced Script System**: Create and execute command sequences with timing
- **Non-blocking Timer System**: Precise delays using millis() without blocking
- **Multi-Interface Control**: Web interface, HTTP API, and Serial command line
- **Automatic Board Detection**: Scans I2C addresses for PCA9685 boards
- **Comprehensive Configuration**: Per-servo settings with persistent storage

## Development Commands

### Build and Upload
```bash
# Build the project (warnings suppressed for faster development)
pio run

# Upload to ESP32 (requires device connected to /dev/ttyUSB0)
pio run --target upload

# Build and upload in one command
pio run --target upload

# Clean build
pio run --target clean
```

### File System Operations
```bash
# Upload web files to SPIFFS
pio run --target uploadfs

# Build filesystem image
pio run --target buildfs
```

### Monitoring
```bash
# Monitor serial output (115200 baud)
pio device monitor

# Monitor with automatic reconnection
pio device monitor --baud 115200 --port /dev/ttyUSB0
```

### Testing
```bash
# Run tests (if any exist)
pio test
```

## Hardware Configuration

- **Target**: ESP32 (esp32dev board)
- **Framework**: Arduino
- **PWM Controllers**: Up to 8 PCA9685 boards (I2C addresses 0x40-0x47)
- **Servos**: Up to 128 servos (16 per board)
- **Upload/Monitor Port**: /dev/ttyUSB0 (115200 baud)
- **Servo Frequency**: 50 Hz

## Code Architecture

### Main Components
- **Multi-Board Support**: Automatic detection and management of multiple PCA9685 boards
- **Servo Configuration**: Comprehensive per-servo configuration with center, range, and initial positions
- **Inverse Pairing**: Support for paired servos with inverse movement relationships
- **Script System**: Create, edit, and execute command sequences with timing
- **Timer System**: Non-blocking command queue with millisecond precision
- **Serial Interface**: Real-time command execution via Serial Monitor (115200 baud)
- **Web Server**: ESPAsyncWebServer provides HTTP endpoints and web interface
- **WiFi Connection**: Connects to WiFi network for web interface access
- **LittleFS**: Stores web interface files, configuration data, and scripts

### Key Constants
- `SERVOMIN/SERVOMAX`: PWM pulse length range (150-600 counts)
- `USMIN/USMAX`: Microsecond pulse range (1000-2000 μs)
- `SERVO_FREQ`: 50 Hz for analog servos
- `MAX_BOARDS`: 8 maximum PCA9685 boards supported
- `SERVOS_PER_BOARD`: 16 servos per PCA9685 board
- `MAX_SCRIPTS`: 20 maximum script actions supported

### Web Interface
- **URL**: Access via ESP32's IP address (displayed in serial monitor)
- **Main Interface**: `/data/index.html` - Servo control and configuration
- **Script Editor**: `/data/scripts.html` - Create and manage scripts
- **API Endpoints**:
  - `GET /api/system` - System information
  - `GET /api/configuration` - Complete servo configuration
  - `POST /api/configuration` - Update servo configuration
  - `POST /api/command` - Execute single command
  - `GET /api/scripts` - List all scripts
  - `POST /api/scripts` - Create new script
  - `PUT /api/scripts` - Update existing script
  - `DELETE /api/scripts` - Delete script
  - `POST /api/scripts/execute` - Execute script
  - `GET /api/debug` - Get debug messages
  - `DELETE /api/debug` - Clear debug log

### Current Behavior
The main loop processes:
1. **Timer Queue**: Executes queued commands with precise timing
2. **Serial Input**: Processes command-line input with prompt (`servo>`)
3. **Web Server**: Handles HTTP requests asynchronously
4. **Configuration**: Loads/saves servo and script configurations to LittleFS

### Command Interface
The system supports multiple command interfaces:

**Serial Interface** (115200 baud):
- Interactive command prompt (`servo>`)
- Backspace support for editing
- Real-time command execution and feedback
- Type `help` for command reference

**Web Interface**:
- Real-time servo control with sliders
- Script editor for creating command sequences
- Configuration management
- Debug console for monitoring

**HTTP API**:
- JSON-based REST API for integration
- All commands available via API endpoints
- Suitable for programmatic control

## Important Notes

- The PCA9685 oscillator frequency is calibrated to 27MHz for accurate PWM timing
- Servo positions are calculated as percentages where 50% = center position
- **WiFi Credentials**: Update `WIFI_SSID` and `WIFI_PASSWORD` in `src/wifi_credentials.h` before uploading
- **LittleFS Upload**: Use `pio run --target uploadfs` to upload web files to ESP32
- **Web Interface**: Access via ESP32's IP address shown in serial monitor during startup

### Script System
- **Commands**: Use semicolon-separated command sequences
- **Timing**: Non-blocking `sleep` commands for precise delays
- **Storage**: Scripts persist to LittleFS filesystem
- **Execution**: Queue-based system prevents blocking main loop
- **Examples**: `servo 0 0 50; sleep 1000; servo 0 0 75`

### Serial Commands
Available commands via serial interface:
- `servo <board> <servo> <position>` - Move servo to position
- `system info|init|save|load` - System operations
- `config <board> <servo> <field> <value>` - Update configuration
- `pair <board1> <servo1> <board2> <servo2>` - Pair servos
- `script <name>` - Execute script by name
- `sleep <milliseconds>` - Non-blocking delay (max 10000ms)
- `help` - Show command reference