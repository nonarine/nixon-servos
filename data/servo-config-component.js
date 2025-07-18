// Servo Configuration Component
// Handles the rendering of individual servo configuration UI elements

class ServoConfigComponent {
    constructor(boardIndex, servo, configuration) {
        this.boardIndex = boardIndex;
        this.servo = servo;
        this.configuration = configuration;
        this.isPaired = servo.isPair;
        this.isPairMaster = servo.isPairMaster;
    }

    render() {
        const servoId = `b${this.boardIndex}s${this.servo.index}`;
        const isEnabled = this.servo.enabled;
        
        // Check if this servo is a slave in a pairing relationship
        // A servo is a slave if there's another servo that:
        // 1. Has isPair=true and isPairMaster=true
        // 2. Points to this servo via pairBoard/pairServo
        const isPairedSlave = this.isSlaveServo();
        
        console.log(`DEBUG: Rendering servo ${this.boardIndex}:${this.servo.index} (${this.servo.name}) - isPairedSlave: ${isPairedSlave}`);
        
        // Don't render controls for paired slave servos
        if (isPairedSlave) {
            console.log(`DEBUG: Hiding slave servo ${this.boardIndex}:${this.servo.index} (${this.servo.name})`);
            return this.renderPairedSlaveIndicator(servoId);
        }
        
        return this.renderFullControls(servoId, isEnabled);
    }

    renderPairedSlaveIndicator(servoId) {
        const masterInfo = this.findMasterServo();
        const masterName = masterInfo ? masterInfo.name : 'Unknown';
        
        return `
            <div class="servo-config paired-slave" id="servo-${servoId}">
                <div class="servo-header">
                    <div class="servo-title">${this.servo.name}</div>
                    <div class="paired-indicator">
                        <span class="paired-label">Paired with: ${masterName}</span>
                    </div>
                </div>
                <div class="paired-slave-info">
                    <p><strong>This servo is controlled by its master servo.</strong></p>
                    <p>All configuration is handled by the master servo. This servo will move inversely to the master.</p>
                    <div class="button-group">
                        <button class="btn btn-danger btn-small" onclick="unpairServo(${this.boardIndex}, ${this.servo.index})">Unpair</button>
                    </div>
                </div>
            </div>
        `;
    }

    renderFullControls(servoId, isEnabled) {
        return `
            <div class="servo-config ${isEnabled ? 'enabled' : ''} ${this.isPaired ? 'paired' : ''}" id="servo-${servoId}">
                <div class="servo-header">
                    <div class="servo-title">${this.servo.name}</div>
                    <label class="servo-enabled">
                        <input type="checkbox" ${isEnabled ? 'checked' : ''} 
                               onchange="updateServoConfig(${this.boardIndex}, ${this.servo.index}, 'enabled', this.checked)">
                        Enabled
                    </label>
                </div>
                
                ${this.renderBasicControls()}
                ${this.renderPairingSection()}
                ${this.renderTestControls()}
            </div>
        `;
    }

    renderBasicControls() {
        return `
            <div class="control-section">
                <h4>Basic Configuration</h4>
                <div class="config-row">
                    <label>Name:</label>
                    <input type="text" value="${this.servo.name}" 
                           onchange="updateServoConfig(${this.boardIndex}, ${this.servo.index}, 'name', this.value)">
                </div>
                
                <div class="config-row">
                    <label>Center:</label>
                    <input type="number" min="0" max="100" step="0.1" value="${this.servo.center}" 
                           onchange="updateServoConfig(${this.boardIndex}, ${this.servo.index}, 'center', parseFloat(this.value))">
                    <span>%</span>
                </div>
                
                <div class="config-row">
                    <label>Range:</label>
                    <input type="number" min="0" max="50" step="0.1" value="${this.servo.range}" 
                           onchange="updateServoConfig(${this.boardIndex}, ${this.servo.index}, 'range', parseFloat(this.value))">
                    <span>%</span>
                </div>
                
                <div class="config-row">
                    <label>Init Position:</label>
                    <input type="number" min="0" max="100" step="0.1" value="${this.servo.initPosition}" 
                           onchange="updateServoConfig(${this.boardIndex}, ${this.servo.index}, 'initPosition', parseFloat(this.value))">
                    <span>%</span>
                </div>
            </div>
        `;
    }

    renderPairingSection() {
        const targetServo = this.getTargetServo();
        const targetServoName = targetServo ? targetServo.name : 'Unknown';
        
        return `
            <div class="control-section">
                <h4>Pairing Configuration</h4>
                <div class="config-row">
                    <label>Enable Pairing:</label>
                    <input type="checkbox" ${this.isPaired ? 'checked' : ''} 
                           onchange="togglePairing(${this.boardIndex}, ${this.servo.index}, this.checked)">
                    <span>Inverse Pair</span>
                </div>
                <div class="pair-config ${this.isPaired ? '' : 'hidden'}" id="pair-b${this.boardIndex}s${this.servo.index}">
                    <h4>Pair Settings</h4>
                    <div class="config-row">
                        <label>Target Board:</label>
                        <select onchange="updateServoConfig(${this.boardIndex}, ${this.servo.index}, 'pairBoard', parseInt(this.value))">
                            ${this.renderBoardOptions(this.servo.pairBoard)}
                        </select>
                    </div>
                    <div class="config-row">
                        <label>Target Servo:</label>
                        <select onchange="updateServoConfig(${this.boardIndex}, ${this.servo.index}, 'pairServo', parseInt(this.value))">
                            ${this.renderServoOptions(this.servo.pairServo)}
                        </select>
                    </div>
                    <div class="config-row">
                        <label>Master Control:</label>
                        <input type="checkbox" ${this.servo.isPairMaster ? 'checked' : ''} 
                               onchange="updateServoConfig(${this.boardIndex}, ${this.servo.index}, 'isPairMaster', this.checked)">
                        <span>This servo controls the pair</span>
                    </div>
                    ${this.servo.isPairMaster && this.servo.pairBoard >= 0 && this.servo.pairServo >= 0 ? `
                        <div class="config-row">
                            <label>Paired Servo:</label>
                            <span class="paired-servo-info">${targetServoName}</span>
                            <button class="btn btn-small btn-danger" onclick="unpairServo(${this.boardIndex}, ${this.servo.index})">Unpair</button>
                        </div>
                    ` : ''}
                    <div class="pair-info">
                        <p><strong>Master:</strong> Controls both servos when moved</p>
                        <p><strong>Slave:</strong> Follows the master servo inversely (shown as paired)</p>
                    </div>
                </div>
            </div>
        `;
    }

    renderTestControls() {
        const minPos = this.servo.center - this.servo.range;
        const maxPos = this.servo.center + this.servo.range;
        
        return `
            <div class="control-section">
                <h4>Test Controls</h4>
                <div class="config-row">
                    <label>Test Position:</label>
                    <input type="range" min="${minPos}" max="${maxPos}" step="0.1" value="${this.servo.center}" 
                           id="testSlider-${this.boardIndex}-${this.servo.index}"
                           oninput="testServoPosition(${this.boardIndex}, ${this.servo.index}, parseFloat(this.value)); updateSliderValue(${this.boardIndex}, ${this.servo.index}, this.value)">
                    <span id="sliderValue-${this.boardIndex}-${this.servo.index}">${this.servo.center}%</span>
                </div>
                <div class="button-group">
                    <button class="btn btn-primary btn-small" onclick="setSliderAndTest(${this.boardIndex}, ${this.servo.index}, ${minPos})">Min</button>
                    <button class="btn btn-primary btn-small" onclick="setSliderAndTest(${this.boardIndex}, ${this.servo.index}, ${this.servo.center})">Center</button>
                    <button class="btn btn-primary btn-small" onclick="setSliderAndTest(${this.boardIndex}, ${this.servo.index}, ${maxPos})">Max</button>
                    <button class="btn btn-warning btn-small" onclick="setSliderAndTest(${this.boardIndex}, ${this.servo.index}, ${this.servo.initPosition})">Init</button>
                </div>
            </div>
        `;
    }

    renderBoardOptions(selectedBoard) {
        let options = '<option value="-1">None</option>';
        if (this.configuration.boards) {
            this.configuration.boards.forEach(board => {
                const selected = board.index === selectedBoard ? 'selected' : '';
                options += `<option value="${board.index}" ${selected}>Board ${board.index} (0x${board.address.toString(16).toUpperCase()})</option>`;
            });
        }
        return options;
    }

    renderServoOptions(selectedServo) {
        let options = '<option value="-1">None</option>';
        for (let i = 0; i < 16; i++) {
            const selected = i === selectedServo ? 'selected' : '';
            options += `<option value="${i}" ${selected}>Servo ${i}</option>`;
        }
        return options;
    }

    isSlaveServo() {
        if (!this.configuration.boards) return false;
        
        // Check if any servo is configured as a master that points to this servo
        for (const board of this.configuration.boards) {
            for (const servo of board.servos) {
                if (servo.isPair && servo.isPairMaster && 
                    servo.pairBoard === this.boardIndex && 
                    servo.pairServo === this.servo.index) {
                    console.log(`DEBUG: Servo ${this.boardIndex}:${this.servo.index} (${this.servo.name}) is a SLAVE - controlled by servo ${board.index}:${servo.index} (${servo.name})`);
                    return true;
                }
            }
        }
        return false;
    }

    findMasterServo() {
        if (!this.configuration.boards) return null;
        
        for (const board of this.configuration.boards) {
            for (const servo of board.servos) {
                if (servo.isPair && servo.isPairMaster && 
                    servo.pairBoard === this.boardIndex && 
                    servo.pairServo === this.servo.index) {
                    return servo;
                }
            }
        }
        return null;
    }

    getTargetServo() {
        if (!this.configuration.boards || this.servo.pairBoard < 0 || this.servo.pairServo < 0) {
            return null;
        }
        
        const targetBoard = this.configuration.boards.find(board => board.index === this.servo.pairBoard);
        if (!targetBoard) return null;
        
        const targetServo = targetBoard.servos.find(servo => servo.index === this.servo.pairServo);
        return targetServo || null;
    }
}