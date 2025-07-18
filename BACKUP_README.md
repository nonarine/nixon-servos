# Servo Configuration Backup System

This system automatically backs up your servo configuration before uploading new filesystem images, preventing loss of servo settings.

## How It Works

When you run `pio run --target uploadfs`, the system:

1. **PRE-ACTION**: Automatically backs up the current configuration from the ESP32
2. **Saves timestamped backups** in the `backups/` directory
3. **Uploads the new filesystem** with your updated web interface
4. **POST-ACTION**: Automatically restores your configuration after upload
5. **Preserves all settings** seamlessly without manual intervention

## Backup Files

- `backups/servo_config_backup_YYYYMMDD_HHMMSS.json` - Timestamped backups
- `backups/servo_config_latest.json` - Always contains the most recent backup

## Manual Backup

To manually backup your configuration:

```bash
python3 backup_config.py
```

## Restoring Configuration

### Automatic Restore (Default)
- **No action required!** Configuration is automatically restored after filesystem upload
- The system waits for ESP32 to restart and then restores all settings
- Success message: "✓ Configuration successfully restored!"

### Manual Restore Methods (if needed)

#### Method 1: Web Interface
1. Open the web interface at `http://192.168.86.68`
2. Click "Load from Offline" button
3. Your configuration will be restored from the ESP32's offline storage

#### Method 2: Manual Restore Tool
```bash
python3 restore_config.py
```

#### Method 3: From Specific Backup
```bash
python3 restore_config.py backups/servo_config_backup_20250718_131045.json
```

## Configuration

The ESP32 IP address can be changed in `platformio.ini`:

```ini
custom_esp32_ip = 192.168.86.68
```

## What Gets Backed Up

- Board configurations and addresses
- Servo enable/disable states
- Servo names and positions (center, range, initial)
- Pairing relationships and master/slave settings
- All custom configuration parameters

## Troubleshooting

### "Could not connect to ESP32"
- Ensure the ESP32 is powered on and connected to WiFi
- Check that the IP address in `platformio.ini` is correct
- Verify the web interface is accessible in your browser

### "No backup found to restore"
- Run the backup script manually first: `python3 backup_config.py`
- Check that the `backups/` directory exists and contains `.json` files

### Backup Failed
- The filesystem upload will still proceed
- Use the "Save to Offline" button in the web interface before uploading
- Manual configuration will be required after upload

## Safety Features

- **Fully Automatic**: Complete backup and restore cycle without user intervention
- **Non-blocking**: Backup failures don't prevent filesystem uploads
- **Timestamped**: Multiple backups are preserved
- **Offline Storage**: ESP32 maintains its own offline configuration
- **Manual Override**: Multiple restore methods available if automatic restore fails
- **Intelligent Retry**: Waits for ESP32 to restart before attempting restore

## Files

- `backup_config.py` - Main backup script (runs automatically)
- `restore_config.py` - Manual restore tool
- `platformio.ini` - Contains backup configuration
- `backups/` - Directory containing all backup files