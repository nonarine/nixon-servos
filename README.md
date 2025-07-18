# ESP32 Multi-Board Servo Controller

A comprehensive ESP32-based servo controller system supporting multiple PCA9685 boards with advanced scripting capabilities and web interface.

## Features

### Hardware Support
- **Multi-Board PCA9685 Support**: Control up to 8 PCA9685 boards (128 servos total)
- **Automatic Board Detection**: Scans common I2C addresses (0x40-0x47)
- **Hot-Swappable Boards**: Detect and configure boards dynamically

### Servo Management
- **Individual Servo Control**: Precise position control (0-100%)
- **Servo Pairing**: Inverse pair servos for synchronized movement
- **Configurable Parameters**: Center position, range limits, initial positions
- **Range Limiting**: Prevent servo damage with configurable safe ranges

### Web Interface
- **Real-time Control**: Browser-based servo control interface
- **Live Configuration**: Modify servo settings without recompiling
- **Script Editor**: Visual script creation and management
- **Debug Console**: Monitor system status and command execution

### Advanced Scripting
- **Script Actions**: Create reusable command sequences
- **Non-blocking Execution**: Scripts run without blocking main loop
- **Timer System**: Precise sleep/delay commands using `millis()`
- **Semicolon Syntax**: Chain commands with `;` separator
- **Filesystem Persistence**: Scripts saved to ESP32 flash memory

### Configuration Management
- **JSON Configuration**: Human-readable configuration format
- **Persistent Storage**: Settings saved to LittleFS filesystem
- **Backup/Restore**: Export and import configurations
- **Offline Mode**: Separate offline configuration support

## Hardware Requirements

- ESP32 Development Board
- One or more PCA9685 16-channel PWM boards
- Servo motors (up to 128 total)
- 5V power supply for servos
- I2C connections (SDA/SCL)

## Wiring

```
ESP32          PCA9685
GPIO 21 (SDA) -> SDA
GPIO 22 (SCL) -> SCL
3.3V          -> VCC
GND           -> GND
```

For multiple boards, daisy-chain the I2C connections and set unique addresses.

## Installation

1. **Clone Repository**
   ```bash
   git clone https://github.com/nonarine/nixon-servos.git
   cd nixon-servos
   ```

2. **Configure WiFi**
   Create `src/wifi_credentials.h`:
   ```cpp
   #pragma once
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourWiFiPassword"
   ```

3. **Install Dependencies**
   - ArduinoJson
   - Adafruit PWM Servo Driver Library
   - ESPAsyncWebServer
   - AsyncTCP

4. **Upload Code**
   - Upload sketch to ESP32
   - Upload filesystem (LittleFS) with web files

## Usage

### Web Interface

1. **Connect to WiFi**: ESP32 will display IP address in Serial Monitor
2. **Open Browser**: Navigate to ESP32's IP address
3. **Configure Servos**: Enable servos and set parameters
4. **Control Servos**: Use sliders for real-time position control

### Serial Interface

The ESP32 provides a command-line interface over Serial (115200 baud):

1. **Connect Serial Monitor**: Open Serial Monitor at 115200 baud
2. **Command Prompt**: Type commands at the `servo>` prompt
3. **Real-time Feedback**: Commands execute immediately with results
4. **Backspace Support**: Use backspace to edit commands
5. **Help Command**: Type `help` for command reference

### Script Editor

1. **Access Scripts**: Click "Script Editor" from main page
2. **Create Script**: Fill in name, description, and commands
3. **Command Syntax**: Use semicolon-separated commands
4. **Execute**: Run scripts immediately or via API

### Command Interface

Commands can be sent via multiple interfaces:

**HTTP API** - POST to `/api/command`:
```json
{
  "command": "servo 0 0 75"
}
```

**Serial Interface** - Type commands at the `servo>` prompt:
```
servo> servo 0 0 75
Executing: servo 0 0 75
Result: Success: Moved servo 0:0 to 75%
servo> 
```

## Command Reference

### Servo Control
```
servo <board> <servo> <position>
```
Move servo to position (0-100%)

### System Commands
```
system info                    # Show system information
system init                    # Apply initial positions
system save                    # Save configuration
system load                    # Load configuration
```

### Configuration
```
config <board> <servo> <field> <value>
```
Update servo configuration fields: `enabled`, `center`, `range`, `initPosition`, `name`

### Servo Pairing
```
pair <board1> <servo1> <board2> <servo2>
```
Pair servos for inverse movement

### Script Commands
```
script <name>                  # Execute script by name
sleep <milliseconds>           # Non-blocking delay (max 10000ms)
```

## Script Examples

### Basic Movement
```
servo 0 0 0; sleep 1000; servo 0 0 100; sleep 1000; servo 0 0 50
```

### Multi-Servo Sequence
```
servo 0 0 75; servo 0 1 25; sleep 500; servo 0 0 50; servo 0 1 50
```

### System Integration
```
system init; sleep 2000; servo 0 0 90; sleep 1000; system save
```

## API Endpoints

### Servo Control
- `GET /api/system` - System information
- `GET /api/configuration` - Current configuration
- `POST /api/configuration` - Update configuration
- `POST /api/command` - Execute command

### Script Management
- `GET /api/scripts` - List all scripts
- `POST /api/scripts` - Create new script
- `PUT /api/scripts` - Update existing script
- `DELETE /api/scripts` - Delete script
- `POST /api/scripts/execute` - Execute script

### Debug
- `GET /api/debug` - Get debug messages
- `DELETE /api/debug` - Clear debug log

## Configuration Format

```json
{
  "boards": [
    {
      "address": 64,
      "name": "PCA9685 @0x40",
      "enabled": true,
      "servos": [
        {
          "enabled": true,
          "center": 50.0,
          "range": 50.0,
          "initPosition": 50.0,
          "name": "Servo Name"
        }
      ]
    }
  ],
  "scripts": [
    {
      "name": "script_name",
      "description": "Script description",
      "commands": "servo 0 0 50; sleep 1000; servo 0 0 75",
      "enabled": true
    }
  ]
}
```

## Timer System

The non-blocking timer system enables precise delays without blocking the main loop:

- **Queue-based**: Commands queued with execution times
- **Millisecond Precision**: Uses Arduino `millis()` for timing
- **Non-blocking**: Main loop continues processing during delays
- **Script Integration**: Sleep commands automatically use timer system

## Development

### Project Structure
```
nixon-servos/
├── src/
│   ├── main.cpp              # Main application
│   ├── ServoController.h/cpp # Core servo control logic
│   ├── WebServer.h/cpp       # HTTP server implementation
│   └── DebugConsole.h/cpp    # Debug logging system
├── data/
│   ├── index.html            # Main web interface
│   ├── scripts.html          # Script editor interface
│   ├── script-editor.js      # Script management JS
│   └── style.css             # Web interface styling
└── README.md
```

### Adding Features

1. **New Commands**: Add to `ServoController::executeCommand()`
2. **Web Interface**: Modify HTML/CSS/JS in `data/` folder
3. **API Endpoints**: Add routes in `WebServer.cpp`
4. **Configuration**: Update JSON serialization methods

## Troubleshooting

### Common Issues

1. **Servo Not Moving**
   - Check power supply (servos need 5V)
   - Verify I2C connections
   - Ensure servo is enabled in configuration
   - Test with serial command: `servo 0 0 50`

2. **Board Not Detected**
   - Check I2C address (default: 0x40)
   - Verify SDA/SCL connections
   - Check power to PCA9685
   - Use serial command: `system info`

3. **Script Not Executing**
   - Verify script is enabled
   - Check command syntax
   - Monitor debug console for errors
   - Test individual commands via serial interface

4. **WiFi Connection Issues**
   - Verify credentials in `wifi_credentials.h`
   - Check WiFi signal strength
   - Monitor Serial output for connection status

5. **Serial Interface Issues**
   - Ensure baud rate is set to 115200
   - Check USB cable connection
   - Verify correct COM port selection
   - Try typing `help` to test connectivity

## License

MIT License - see LICENSE file for details.

## Contributing

1. Fork the repository
2. Create feature branch
3. Commit changes
4. Push to branch
5. Create Pull Request

## Support

For issues and questions:
- GitHub Issues: [nonarine/nixon-servos/issues](https://github.com/nonarine/nixon-servos/issues)
- Check Serial Monitor for debug output
- Review debug console in web interface