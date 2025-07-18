# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a PlatformIO-based ESP32 project that controls servo motors using multiple PCA9685 I2C PWM controllers. The system supports up to 8 PCA9685 boards (128 servos total) with automatic board detection and comprehensive servo configuration through a web interface.

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
- **Board Scanning**: Automatic I2C scanning to detect connected PCA9685 boards
- **Web Server**: ESPAsyncWebServer provides HTTP endpoints for servo configuration
- **WiFi Connection**: Connects to WiFi network for web interface access
- **LittleFS**: Stores web interface files and configuration data

### Key Constants
- `SERVOMIN/SERVOMAX`: PWM pulse length range (150-600 counts)
- `USMIN/USMAX`: Microsecond pulse range (1000-2000 μs)
- `SERVO_FREQ`: 50 Hz for analog servos
- `MAX_BOARDS`: 8 maximum PCA9685 boards supported
- `SERVOS_PER_BOARD`: 16 servos per PCA9685 board

### Web Interface
- **URL**: Access via ESP32's IP address (displayed in serial monitor)
- **Files**: Web interface stored in `/data/index.html`
- **API Endpoints**:
  - `POST /api/servo` - Set individual servo position
  - `POST /api/servos` - Set multiple servo positions
  - `GET /api/config` - Get current servo positions

### Current Behavior
The main loop creates a sweeping motion on servos 0 and 1, alternating between opposite directions every 600ms with ±20% deviation from center position. The web interface allows real-time control of all 16 servos.

## Important Notes

- The PCA9685 oscillator frequency is calibrated to 27MHz for accurate PWM timing
- Servo positions are calculated as percentages where 50% = center position
- The code includes detailed comments about oscillator calibration requirements
- **WiFi Credentials**: Update `WIFI_SSID` and `WIFI_PASSWORD` in `src/wifi_credentials.h` before uploading
- **SPIFFS Upload**: Use `pio run --target uploadfs` to upload web files to ESP32
- **Web Interface**: Access via ESP32's IP address shown in serial monitor during startup