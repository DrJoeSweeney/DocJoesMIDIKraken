/**
 * ESP32 Peripheral Node Firmware
 *
 * Scans encoders and buttons using shift registers
 * Sends events to Teensy via I2C slave interface
 * Uses dual-core architecture for maximum scan rate
 */

#include <Arduino.h>
#include <Protocol.h>
#include <ShiftRegisterDMA.h>
#include <EncoderDecoder.h>
#include <ButtonHandler.h>
#include <LockFreeQueue.h>
#include <I2CSlave.h>
#include <Diagnostics.h>

// ============================================================================
// CONFIGURATION
// ============================================================================
// NOTE: This configuration is for a 32-encoder synth panel (8 total in system)
//
// I2C Address Assignment (9 I2C ESP32 nodes total):
// - Bus 0 (Wire):  Synth #1-3 = 0x08, 0x09, 0x0A
// - Bus 1 (Wire1): Synth #4-6 = 0x0B, 0x0C, 0x0D
// - Bus 2 (Wire2): Synth #7-8 = 0x0E, 0x0F | FX #9 = 0x10
//
// Panel Configurations:
// - Synth panels (#1-8): 32 encoders, 32 encoder buttons, 4 standalone = 13 shift registers
// - FX panel (#9): 28 encoders, 28 encoder buttons = 11 shift registers
// - Snapshot panel: 19 buttons, handled via separate interface (not I2C)
//
// IMPORTANT: Set I2C_ADDRESS below to match the physical panel position (0x08-0x10)

// I2C Configuration
#define I2C_ADDRESS 0x08
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_EVENT_PIN 19

// Shift Register Configuration
#define SR_MISO_PIN 12
#define SR_SCK_PIN 14
#define SR_LATCH_PIN 27
#define SR_NUM_CHIPS 13  // 32-encoder synth panel: 100 bits = 13 chips

// Control Configuration (32-encoder synth panel)
// Layout: 32 encoders (64 bits) + 32 encoder buttons + 4 standalone buttons (36 bits)
// Total: 100 bits across 13 shift registers
#define NUM_ENCODERS 32
#define NUM_BUTTONS 36  // 32 encoder buttons + 4 standalone buttons
#define ENCODER_OFFSET 0
#define BUTTON_OFFSET 64

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

ShiftRegisterDMA shiftReg(VSPI_HOST, SR_MISO_PIN, SR_SCK_PIN, SR_LATCH_PIN, SR_NUM_CHIPS);
EncoderDecoder encoders(NUM_ENCODERS);
ButtonHandler buttons(NUM_BUTTONS);
LockFreeQueue<EventMessage> eventQueue(128);
I2CSlave i2cSlave(I2C_ADDRESS, Wire, I2C_EVENT_PIN);
Diagnostics diagnostics;

// ============================================================================
// CORE 0 - SCANNER TASK (High Priority)
// ============================================================================

void core0_scanner_task(void* param) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t scanInterval = pdMS_TO_TICKS(1000 / 5000);  // 5kHz scan rate (32-encoder panel)

    while (1) {
        uint32_t cycleStart = micros();

        // Start DMA read
        shiftReg.startDMA();

        // Wait for DMA completion
        while (!shiftReg.isDMAComplete()) {
            taskYIELD();
        }

        // Get shift register data
        const uint8_t* data = shiftReg.getDMABuffer();

        // Update encoders (first half of data)
        encoders.update(data);

        // Update buttons (second half of data)
        buttons.update(data + ENCODER_OFFSET, BUTTON_OFFSET);

        // Generate encoder events
        for (uint16_t i = 0; i < NUM_ENCODERS; i++) {
            int8_t delta = encoders.getDelta(i);
            if (delta != 0) {
                EventMessage event;
                event.globalID = i;
                event.value = delta;
                event.flags = (delta > 0) ? EVENT_FLAG_ENCODER_CW : EVENT_FLAG_ENCODER_CCW;
                event.timestamp = micros();

                if (!eventQueue.push(event)) {
                    diagnostics.recordEvent(true);  // Dropped
                }
            }
        }

        // Generate button events
        for (uint16_t i = 0; i < NUM_BUTTONS; i++) {
            if (buttons.isPressed(i)) {
                EventMessage event;
                event.globalID = NUM_ENCODERS + i;
                event.value = 127;
                event.flags = EVENT_FLAG_BUTTON_PRESSED;
                event.timestamp = micros();

                if (!eventQueue.push(event)) {
                    diagnostics.recordEvent(true);
                }
            }

            if (buttons.isReleased(i)) {
                EventMessage event;
                event.globalID = NUM_ENCODERS + i;
                event.value = 0;
                event.flags = EVENT_FLAG_BUTTON_RELEASED;
                event.timestamp = micros();

                if (!eventQueue.push(event)) {
                    diagnostics.recordEvent(true);
                }
            }
        }

        uint32_t cycleTime = micros() - cycleStart;
        diagnostics.recordScanCycle(cycleTime);

        // Wait for next scan cycle
        vTaskDelayUntil(&lastWakeTime, scanInterval);
    }
}

// ============================================================================
// CORE 1 - COMMUNICATION TASK
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("MIDI Kraken ESP32 Peripheral Node");
    Serial.printf("I2C Address: 0x%02X\n", I2C_ADDRESS);

    // Initialize shift registers
    if (!shiftReg.begin(1000000)) {  // 1MHz SPI clock
        Serial.println("ERROR: Failed to initialize shift registers!");
        while (1) { delay(1000); }
    }
    Serial.println("Shift registers initialized");

    // Initialize decoders
    encoders.begin();
    buttons.begin(true);  // Active low
    Serial.println("Encoders and buttons initialized");

    // Initialize I2C slave
    if (!i2cSlave.begin(I2C_SDA_PIN, I2C_SCL_PIN, 1000000)) {
        Serial.println("ERROR: Failed to initialize I2C slave!");
        while (1) { delay(1000); }
    }
    Serial.println("I2C slave initialized");

    // Initialize diagnostics
    diagnostics.begin();
    Serial.println("Diagnostics initialized");

    // Start Core 0 scanner task
    xTaskCreatePinnedToCore(
        core0_scanner_task,
        "Scanner",
        8192,
        NULL,
        1,  // High priority
        NULL,
        0   // Core 0
    );
    Serial.println("Core 0 scanner task started");

    Serial.println("ESP32 Peripheral Node ready!");
}

void loop() {
    // Core 1: Handle I2C communication and event transfer

    // Transfer events from queue to I2C slave
    EventMessage event;
    while (eventQueue.pop(event)) {
        if (i2cSlave.queueEvent(event)) {
            diagnostics.recordEvent(false);
        } else {
            diagnostics.recordEvent(true);  // I2C queue full
        }
    }

    // Update I2C slave
    i2cSlave.update();

    // Update diagnostics
    diagnostics.update();

    // Print diagnostics every 5 seconds
    static uint32_t lastDiagnostics = 0;
    if (millis() - lastDiagnostics > 5000) {
        diagnostics.printDiagnostics();
        lastDiagnostics = millis();
    }

    delay(1);  // Small delay to prevent watchdog
}
