#!/usr/bin/env python3
"""
Manual script to restore servo configuration from backup.
Usage: python restore_config.py [backup_file] [esp32_ip]
"""

import sys
import json
import requests
import time

def restore_configuration(backup_file="backups/servo_config_latest.json", esp32_ip="192.168.86.68"):
    """Restore configuration from backup file."""
    
    try:
        print(f"Loading configuration from {backup_file}...")
        with open(backup_file, 'r') as f:
            config_data = json.load(f)
        
        print(f"Checking ESP32 at {esp32_ip}...")
        
        # Check if ESP32 is responsive
        response = requests.get(f"http://{esp32_ip}/api/info", timeout=10)
        if response.status_code != 200:
            print(f"ESP32 not responding properly (HTTP {response.status_code})")
            return False
        
        print("ESP32 is responsive")
        print("\nTo restore the configuration:")
        print("1. Open the web interface in your browser")
        print(f"2. Go to http://{esp32_ip}")
        print("3. Click 'Load from Offline' button")
        print("\nAlternatively, you can manually configure servos using the backed up settings:")
        
        # Show summary of backed up configuration
        if config_data.get('success') and config_data.get('boards'):
            print(f"\nBacked up configuration summary:")
            for board in config_data['boards']:
                enabled_servos = sum(1 for servo in board['servos'] if servo['enabled'])
                paired_servos = sum(1 for servo in board['servos'] if servo['isPair'])
                print(f"  Board {board['index']} (0x{board['address']:02X}): {enabled_servos} enabled servos, {paired_servos} paired")
        
        return True
        
    except FileNotFoundError:
        print(f"Backup file not found: {backup_file}")
        print("Available backups:")
        import os
        if os.path.exists("backups"):
            for f in os.listdir("backups"):
                if f.endswith('.json'):
                    print(f"  backups/{f}")
        return False
        
    except Exception as e:
        print(f"Error restoring configuration: {str(e)}")
        return False

if __name__ == "__main__":
    backup_file = "backups/servo_config_latest.json"
    esp32_ip = "192.168.86.68"
    
    if len(sys.argv) > 1:
        backup_file = sys.argv[1]
    if len(sys.argv) > 2:
        esp32_ip = sys.argv[2]
    
    print("Servo Configuration Restore Tool")
    print("=" * 40)
    
    success = restore_configuration(backup_file, esp32_ip)
    
    if success:
        print("\nRestore instructions provided above.")
    else:
        print("\nRestore failed. Check the errors above.")
        sys.exit(1)