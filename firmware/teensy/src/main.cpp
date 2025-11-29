/**
 * Teensy 4.0 Main Controller Firmware
 *
 * Receives events from 11 ESP32s (9 on I2C + 1 WiFi on UART)
 * - 8 × 32-encoder synth panels (I2C 0x08-0x0F)
 * - 1 × FX panel (I2C 0x10)
 * - 1 × Snapshot panel (shared with Bus 2 or separate UART)
 * - 1 × WiFi module (UART)
 *
 * 3 I2C Buses:
 * - Bus 0: ESP32 #1, #2, #3 (0x08, 0x09, 0x0A)
 * - Bus 1: ESP32 #4, #5, #6 (0x0B, 0x0C, 0x0D)
 * - Bus 2: ESP32 #7, #8, #9 (0x0E, 0x0F, 0x10)
 *
 * Manages state, snapshots, and sessions
 * Generates and sends USB MIDI messages
 */

#include <Arduino.h>
#include <usb_midi.h>
#include <Protocol.h>
#include <MultiI2CMaster.h>
#include <StateManager.h>
#include <MIDIEngine.h>
#include <Joystick.h>
#include <Diagnostics.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// Joystick Configuration
#define JOYSTICK_X_PIN A0
#define JOYSTICK_Y_PIN A1
#define JOYSTICK_BTN_PIN 2

// Event Interrupt Pin
#define EVENT_INT_PIN 3

// LED Pin
#define LED_PIN 13

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

MultiI2CMaster i2cMaster;
StateManager stateManager;
MIDIEngine midiEngine;
Joystick joystick(JOYSTICK_X_PIN, JOYSTICK_Y_PIN, JOYSTICK_BTN_PIN);
Diagnostics diagnostics;

// Statistics
uint32_t eventsProcessed = 0;
uint32_t midiMessagesSent = 0;
uint32_t lastStatsTime = 0;

// ============================================================================
// INTERRUPT HANDLER
// ============================================================================

void onEventInterrupt() {
    // ESP32 has events ready - poll immediately
    i2cMaster.poll();
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("MIDI Kraken - Teensy 4.0 Main Controller");
    Serial.println("==========================================");

    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    // Initialize I2C master (3 buses)
    if (!i2cMaster.begin(1000000)) {  // 1MHz Fast Mode+
        Serial.println("ERROR: Failed to initialize I2C master!");
        while (1) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            delay(100);
        }
    }
    Serial.println("I2C master initialized (3 buses)");

    // Attach interrupt for event notifications
    i2cMaster.attachInterrupt(EVENT_INT_PIN, onEventInterrupt);
    Serial.println("Event interrupt attached");

    // Initialize state manager
    stateManager.begin();
    Serial.println("State manager initialized (619 controls)");

    // Initialize MIDI engine
    midiEngine.begin();
    Serial.println("MIDI engine initialized (4 virtual devices)");

    // Initialize joystick
    joystick.begin();
    joystick.setDeadZone(10);
    Serial.println("Joystick initialized");

    // Initialize diagnostics
    diagnostics.begin();
    Serial.println("Diagnostics initialized");

    // Check ESP32 slave health (9 peripheral ESP32s on I2C)
    Serial.println("\nChecking ESP32 slave health:");
    for (uint8_t addr = 0x08; addr <= 0x10; addr++) {
        const char* nodeType;
        if (addr <= 0x0F) {
            nodeType = "Synth Panel";
        } else if (addr == 0x10) {
            nodeType = "FX Panel";
        } else {
            nodeType = "Unknown";
        }

        bool healthy = i2cMaster.isSlaveHealthy(addr);
        Serial.printf("  ESP32 #%d (0x%02X) [%s]: %s\n",
            addr - 0x07, addr, nodeType, healthy ? "OK" : "FAIL");
    }
    Serial.println("  ESP32 #11 (WiFi): UART interface");

    digitalWrite(LED_PIN, LOW);
    Serial.println("\nTeensy Main Controller ready!");
    Serial.println("Listening for MIDI events...\n");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    uint32_t loopStart = micros();

    // Poll I2C buses for events (if not triggered by interrupt)
    static uint32_t lastPoll = 0;
    if (micros() - lastPoll > 1000) {  // Poll every 1ms if no interrupts
        i2cMaster.poll();
        lastPoll = micros();
    }

    // Process events from I2C master
    EventMessage event;
    while (i2cMaster.getEvent(event)) {
        eventsProcessed++;

        // Update state
        stateManager.setValue(event.globalID, event.value);

        // Get control configuration
        const ControlConfig* config = stateManager.getConfig(event.globalID);
        if (config && (config->flags & CONTROL_FLAG_ENABLED)) {
            // Send MIDI
            if (midiEngine.processControl(*config, event.value)) {
                midiMessagesSent++;
            }
        }

        diagnostics.recordEvent(false);
    }

    // Process joystick
    joystick.update();

    // Send pitch bend from joystick X-axis
    static uint16_t lastPitch = 8192;
    uint16_t currentPitch = joystick.getPitchBend();
    if (abs((int)currentPitch - (int)lastPitch) > 50) {  // Threshold
        midiEngine.sendPitchBend(0, 0, currentPitch);  // Device 0, Channel 0
        lastPitch = currentPitch;
        midiMessagesSent++;
    }

    // Send modulation from joystick Y-axis
    static uint8_t lastMod = 64;
    uint8_t currentMod = joystick.getModulation();
    if (abs((int)currentMod - (int)lastMod) > 2) {  // Threshold
        midiEngine.sendCC(0, 0, 1, currentMod);  // CC 1 = Modulation
        lastMod = currentMod;
        midiMessagesSent++;
    }

    // Joystick button - send all notes off (panic)
    if (joystick.isButtonPressed()) {
        Serial.println("Panic! Sending all notes off...");
        for (uint8_t dev = 0; dev < 4; dev++) {
            midiEngine.sendAllNotesOff(dev);
        }
    }

    // Read MIDI from USB (for MIDI learn, etc.)
    while (usbMIDI.read()) {
        // Handle incoming MIDI if needed
    }

    // Update diagnostics
    uint32_t loopTime = micros() - loopStart;
    diagnostics.recordScanCycle(loopTime);
    diagnostics.update();

    // Print statistics every 5 seconds
    if (millis() - lastStatsTime > 5000) {
        Serial.println("=== Statistics ===");
        Serial.printf("Events processed: %u\n", eventsProcessed);
        Serial.printf("MIDI messages sent: %u\n", midiMessagesSent);
        Serial.printf("MIDI message rate: %.1f msg/sec\n", midiEngine.getMessageRate());
        Serial.printf("I2C event queue: %u\n", i2cMaster.getQueuedEventCount());
        Serial.printf("Loop time: %u us\n", loopTime);
        Serial.println();

        diagnostics.printDiagnostics();

        lastStatsTime = millis();
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Heartbeat
    }

    // Small delay to prevent overwhelming the system
    delayMicroseconds(100);
}
