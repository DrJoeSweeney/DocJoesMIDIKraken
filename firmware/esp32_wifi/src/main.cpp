/**
 * ESP32 WiFi Node Firmware (ESP32 #7)
 *
 * Provides web-based configuration interface
 * Stores sessions on SD card
 * Also handles scanning for one encoder/button section
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>
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

// WiFi Configuration
#define WIFI_SSID "MIDI_Kraken"
#define WIFI_PASSWORD "midicontrol"
#define WIFI_CHANNEL 6

// I2C Configuration
#define I2C_ADDRESS 0x0E
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_EVENT_PIN 19

// SD Card Configuration
#define SD_CS_PIN 5
#define SD_MOSI_PIN 23
#define SD_MISO_PIN 19
#define SD_SCK_PIN 18

// Shift Register Configuration
#define SR_MISO_PIN 12
#define SR_SCK_PIN 14
#define SR_LATCH_PIN 27
#define SR_NUM_CHIPS 16

// Control Configuration
#define NUM_ENCODERS 32
#define NUM_BUTTONS 64

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

WebServer webServer(80);
ShiftRegisterDMA shiftReg(VSPI_HOST, SR_MISO_PIN, SR_SCK_PIN, SR_LATCH_PIN, SR_NUM_CHIPS);
EncoderDecoder encoders(NUM_ENCODERS);
ButtonHandler buttons(NUM_BUTTONS);
LockFreeQueue<EventMessage> eventQueue(128);
I2CSlave i2cSlave(I2C_ADDRESS, Wire, I2C_EVENT_PIN);
Diagnostics diagnostics;

bool sdCardPresent = false;

// ============================================================================
// WEB SERVER HANDLERS
// ============================================================================

void handleRoot() {
    String html = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>MIDI Kraken Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #1a1a1a; color: #fff; }
        h1 { color: #4CAF50; }
        .card { background: #2a2a2a; padding: 20px; margin: 10px 0; border-radius: 8px; }
        .button { background: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; }
        .button:hover { background: #45a049; }
    </style>
</head>
<body>
    <h1>MIDI Kraken Configuration</h1>
    <div class="card">
        <h2>System Status</h2>
        <p>WiFi: Connected</p>
        <p>SD Card: )HTML" + String(sdCardPresent ? "Present" : "Not Found") + R"HTML(</p>
        <p>I2C Address: 0x0E</p>
        <p>Firmware: v1.0.0</p>
    </div>
    <div class="card">
        <h2>Quick Actions</h2>
        <button class="button" onclick="location.href='/sessions'">Manage Sessions</button>
        <button class="button" onclick="location.href='/controls'">Configure Controls</button>
        <button class="button" onclick="location.href='/diagnostics'">View Diagnostics</button>
    </div>
    <div class="card">
        <h2>About</h2>
        <p>MIDI Kraken - 619-control MIDI controller</p>
        <p>For documentation, visit: <a href="https://github.com/yourusername/DocJoesMIDIKraken" style="color: #4CAF50;">GitHub</a></p>
    </div>
</body>
</html>
)HTML";

    webServer.send(200, "text/html", html);
}

void handleSessions() {
    webServer.send(200, "text/html", "<h1>Session Management</h1><p>Coming soon...</p>");
}

void handleControls() {
    webServer.send(200, "text/html", "<h1>Control Configuration</h1><p>Coming soon...</p>");
}

void handleDiagnostics() {
    String json = "{";
    json += "\"eventsProcessed\":" + String(diagnostics.getMetrics().eventsProcessed) + ",";
    json += "\"eventsDropped\":" + String(diagnostics.getMetrics().eventsDropped) + ",";
    json += "\"scanCycleTime\":" + String(diagnostics.getMetrics().scanCycleTime) + ",";
    json += "\"avgScanCycleTime\":" + String(diagnostics.getMetrics().avgScanCycleTime) + ",";
    json += "\"maxScanCycleTime\":" + String(diagnostics.getMetrics().maxScanCycleTime);
    json += "}";

    webServer.send(200, "application/json", json);
}

void handleNotFound() {
    webServer.send(404, "text/plain", "404: Not Found");
}

// ============================================================================
// CORE 0 - SCANNER TASK
// ============================================================================

void core0_scanner_task(void* param) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t scanInterval = pdMS_TO_TICKS(1000 / 3000);  // 3kHz

    while (1) {
        uint32_t cycleStart = micros();

        shiftReg.startDMA();
        while (!shiftReg.isDMAComplete()) {
            taskYIELD();
        }

        const uint8_t* data = shiftReg.getDMABuffer();

        encoders.update(data);
        buttons.update(data, NUM_ENCODERS * 2);

        // Generate events (same as peripheral node)
        for (uint16_t i = 0; i < NUM_ENCODERS; i++) {
            int8_t delta = encoders.getDelta(i);
            if (delta != 0) {
                EventMessage event = {i, (uint8_t)abs(delta),
                    (uint8_t)((delta > 0) ? EVENT_FLAG_ENCODER_CW : EVENT_FLAG_ENCODER_CCW),
                    micros()};
                eventQueue.push(event);
            }
        }

        uint32_t cycleTime = micros() - cycleStart;
        diagnostics.recordScanCycle(cycleTime);

        vTaskDelayUntil(&lastWakeTime, scanInterval);
    }
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("MIDI Kraken ESP32 WiFi Node");
    Serial.println("============================");

    // Initialize SD card
    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    if (SD.begin(SD_CS_PIN)) {
        sdCardPresent = true;
        Serial.println("SD card initialized");
    } else {
        Serial.println("WARNING: SD card not found!");
    }

    // Initialize shift registers
    if (!shiftReg.begin(1000000)) {
        Serial.println("ERROR: Failed to initialize shift registers!");
    } else {
        Serial.println("Shift registers initialized");
    }

    // Initialize decoders
    encoders.begin();
    buttons.begin(true);
    Serial.println("Encoders and buttons initialized");

    // Initialize I2C slave
    if (!i2cSlave.begin(I2C_SDA_PIN, I2C_SCL_PIN, 1000000)) {
        Serial.println("ERROR: Failed to initialize I2C slave!");
    } else {
        Serial.println("I2C slave initialized");
    }

    // Initialize diagnostics
    diagnostics.begin();

    // Start WiFi Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("WiFi AP started: ");
    Serial.println(WIFI_SSID);
    Serial.print("IP Address: ");
    Serial.println(IP);

    // Setup web server routes
    webServer.on("/", handleRoot);
    webServer.on("/sessions", handleSessions);
    webServer.on("/controls", handleControls);
    webServer.on("/diagnostics", handleDiagnostics);
    webServer.onNotFound(handleNotFound);

    webServer.begin();
    Serial.println("Web server started");

    // Start Core 0 scanner task
    xTaskCreatePinnedToCore(core0_scanner_task, "Scanner", 8192, NULL, 1, NULL, 0);
    Serial.println("Core 0 scanner task started");

    Serial.println("\nESP32 WiFi Node ready!");
    Serial.println("Connect to WiFi: " + String(WIFI_SSID));
    Serial.println("Browse to: http://" + IP.toString());
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    // Handle web server
    webServer.handleClient();

    // Transfer events from queue to I2C slave
    EventMessage event;
    while (eventQueue.pop(event)) {
        i2cSlave.queueEvent(event);
    }

    // Update I2C slave
    i2cSlave.update();

    // Update diagnostics
    diagnostics.update();

    delay(1);
}
