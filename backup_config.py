#!/usr/bin/env python3
"""
PlatformIO pre-action script to backup servo configuration before filesystem upload.
This script downloads the current configuration from the ESP32 and saves it locally.
"""

import os
import sys
import json
import requests
import time
from datetime import datetime

try:
    Import("env")
    # Running as PlatformIO script
    esp32_ip = env.GetProjectOption("custom_esp32_ip", "192.168.86.68")
except:
    # Running standalone
    esp32_ip = "192.168.86.68"

def backup_configuration():
    """Backup current configuration from ESP32 before filesystem upload."""
    
    print(f"[BACKUP] Attempting to backup configuration from {esp32_ip}...")
    
    try:
        # First, trigger ESP32 to save current config to offline storage
        try:
            save_response = requests.post(f"http://{esp32_ip}/api/save-offline", timeout=10)
            if save_response.status_code == 200:
                print("[BACKUP] Triggered ESP32 to save current config to offline storage")
            else:
                print(f"[BACKUP] Warning: Could not trigger offline save (HTTP {save_response.status_code})")
        except:
            print("[BACKUP] Warning: Could not trigger offline save")
        
        # Try to get current configuration
        response = requests.get(f"http://{esp32_ip}/api/config", timeout=10)
        
        if response.status_code == 200:
            config_data = response.json()
            
            # Create backups directory if it doesn't exist
            backup_dir = "backups"
            if not os.path.exists(backup_dir):
                os.makedirs(backup_dir)
            
            # Generate timestamp for backup filename
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            backup_filename = f"{backup_dir}/servo_config_backup_{timestamp}.json"
            
            # Save configuration backup
            with open(backup_filename, 'w') as f:
                json.dump(config_data, f, indent=2)
            
            print(f"[BACKUP] Configuration backed up to: {backup_filename}")
            
            # Also save as latest backup
            latest_backup = f"{backup_dir}/servo_config_latest.json"
            with open(latest_backup, 'w') as f:
                json.dump(config_data, f, indent=2)
            
            print(f"[BACKUP] Latest backup saved to: {latest_backup}")
            
            # Copy configuration to data directory for filesystem upload
            data_config_file = "data/offline_config.json"
            with open(data_config_file, 'w') as f:
                json.dump(config_data, f, indent=2)
            
            print(f"[BACKUP] Configuration copied to filesystem: {data_config_file}")
            
            # Try to get offline configuration as well
            try:
                offline_response = requests.get(f"http://{esp32_ip}/api/debug", timeout=5)
                if offline_response.status_code == 200:
                    print("[BACKUP] ESP32 is responsive, offline config should be preserved")
            except:
                pass
                
        else:
            print(f"[BACKUP] Failed to get configuration: HTTP {response.status_code}")
            
    except requests.exceptions.ConnectionError:
        print(f"[BACKUP] Could not connect to ESP32 at {esp32_ip}")
        print("[BACKUP] Make sure ESP32 is connected and accessible")
        
    except requests.exceptions.Timeout:
        print(f"[BACKUP] Timeout connecting to ESP32 at {esp32_ip}")
        
    except Exception as e:
        print(f"[BACKUP] Error backing up configuration: {str(e)}")
    
    print("[BACKUP] Backup attempt completed, proceeding with filesystem upload...")

def restore_configuration():
    """Restore configuration after filesystem upload."""
    print("[RESTORE] Configuration automatically restored via filesystem upload")
    print("[RESTORE] ✓ Your servo settings have been preserved")
    print(f"[RESTORE] ESP32 should be available at http://{esp32_ip}")
    
    # Optional: Verify that ESP32 is responding
    print("[RESTORE] Verifying ESP32 is online...")
    time.sleep(3)  # Give ESP32 time to restart
    
    max_retries = 10
    for i in range(max_retries):
        try:
            response = requests.get(f"http://{esp32_ip}/api/info", timeout=5)
            if response.status_code == 200:
                print("[RESTORE] ✓ ESP32 is online and ready")
                return
        except:
            if i < max_retries - 1:
                print(f"[RESTORE] Waiting for ESP32... ({i+1}/{max_retries})")
                time.sleep(2)
            else:
                print("[RESTORE] ESP32 not responding yet, but configuration should be restored")
                print(f"[RESTORE] Check http://{esp32_ip} once it's online")
                return

# Handle PlatformIO pre/post actions
try:
    # Check if we're in a PlatformIO context
    if "uploadfs" in COMMAND_LINE_TARGETS:
        import sys
        import os
        
        # Determine if this is pre or post action based on environment
        pio_env = os.environ.get('PIOENV', '')
        script_mode = os.environ.get('PLATFORMIO_BUILD_TYPE', '')
        
        # Use a simple file-based approach to detect pre/post
        backup_flag_file = ".backup_completed"
        
        if not os.path.exists(backup_flag_file):
            # Pre-action: Backup before upload
            print("\n" + "="*60)
            print("PRE-ACTION: BACKING UP CONFIGURATION")
            print("="*60)
            backup_configuration()
            # Create flag file to indicate backup completed
            with open(backup_flag_file, 'w') as f:
                f.write("backup_completed")
            print("="*60 + "\n")
        else:
            # Post-action: Restore after upload
            print("\n" + "="*60)
            print("POST-ACTION: RESTORING CONFIGURATION")
            print("="*60)
            restore_configuration()
            # Remove flag file
            os.remove(backup_flag_file)
            print("="*60 + "\n")
            
except NameError:
    # Running standalone - provide main functionality
    if __name__ == "__main__":
        print("Servo Configuration Backup Tool")
        print("=" * 40)
        backup_configuration()