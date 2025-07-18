// Servo Pairing Logic
// Handles pairing/unpairing operations and pairing state management

function togglePairing(boardIndex, servoIndex, enabled) {
    updateServoConfig(boardIndex, servoIndex, 'isPair', enabled);
    
    // Update the local configuration
    if (configuration.boards && configuration.boards[boardIndex] && 
        configuration.boards[boardIndex].servos && configuration.boards[boardIndex].servos[servoIndex]) {
        configuration.boards[boardIndex].servos[servoIndex].isPair = enabled;
        
        // If disabling pairing, also reset pairing settings
        if (!enabled) {
            configuration.boards[boardIndex].servos[servoIndex].isPairMaster = false;
            configuration.boards[boardIndex].servos[servoIndex].pairBoard = -1;
            configuration.boards[boardIndex].servos[servoIndex].pairServo = -1;
        }
    }
    
    // Re-render the configuration to update the UI
    setTimeout(() => {
        renderConfiguration();
    }, 500); // Small delay to allow the server config to update
}

function unpairServo(boardIndex, servoIndex) {
    console.log(`DEBUG: unpairServo called for ${boardIndex}:${servoIndex}`);
    
    // If this is called on a slave servo, we need to unpair the master
    const servo = configuration.boards[boardIndex].servos[servoIndex];
    const component = new ServoConfigComponent(boardIndex, servo, configuration);
    const masterServo = component.findMasterServo();
    
    if (masterServo) {
        console.log(`DEBUG: Unpairing master servo that controls this slave`);
        // Find the master servo's board index
        for (let bIndex = 0; bIndex < configuration.boards.length; bIndex++) {
            for (let sIndex = 0; sIndex < configuration.boards[bIndex].servos.length; sIndex++) {
                if (configuration.boards[bIndex].servos[sIndex] === masterServo) {
                    updateServoConfig(bIndex, sIndex, 'isPair', false);
                    updateServoConfig(bIndex, sIndex, 'isPairMaster', false);
                    updateServoConfig(bIndex, sIndex, 'pairBoard', -1);
                    updateServoConfig(bIndex, sIndex, 'pairServo', -1);
                    break;
                }
            }
        }
    } else {
        console.log(`DEBUG: Unpairing this servo (it's a master)`);
        // This servo is the master, unpair it normally
        updateServoConfig(boardIndex, servoIndex, 'isPair', false);
        updateServoConfig(boardIndex, servoIndex, 'isPairMaster', false);
        updateServoConfig(boardIndex, servoIndex, 'pairBoard', -1);
        updateServoConfig(boardIndex, servoIndex, 'pairServo', -1);
    }
    
    // Refresh the configuration to update the UI
    setTimeout(() => loadConfiguration(), 1000);
}

// Helper function to check if a servo is a slave (controlled by another servo)
function isSlaveServo(boardIndex, servoIndex, configuration) {
    if (!configuration.boards) return false;
    
    // Check if any servo is configured as a master that points to this servo
    for (const board of configuration.boards) {
        for (const servo of board.servos) {
            if (servo.isPair && servo.isPairMaster && 
                servo.pairBoard === boardIndex && 
                servo.pairServo === servoIndex) {
                return true;
            }
        }
    }
    return false;
}

// Helper function to find the master servo that controls a given servo
function findMasterServo(boardIndex, servoIndex, configuration) {
    if (!configuration.boards) return null;
    
    for (const board of configuration.boards) {
        for (const servo of board.servos) {
            if (servo.isPair && servo.isPairMaster && 
                servo.pairBoard === boardIndex && 
                servo.pairServo === servoIndex) {
                return servo;
            }
        }
    }
    return null;
}