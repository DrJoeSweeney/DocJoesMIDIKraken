# I2C Address Reference - MIDI Kraken

**Version**: 1.0
**Date**: 2025-01-29
**For**: 32-Encoder Panel Design (v3.0)

---

## Quick Reference Table

| Panel ID | Panel Type | I2C Address | Teensy Bus | ESP32 Config | Shift Registers | Total Controls |
|----------|------------|-------------|------------|--------------|-----------------|----------------|
| #1 | Synth Panel A1 | **0x08** | Bus 0 (Wire) | Synth | 13 | 32E + 32EB + 4B |
| #2 | Synth Panel A2 | **0x09** | Bus 0 (Wire) | Synth | 13 | 32E + 32EB + 4B |
| #3 | Synth Panel B1 | **0x0A** | Bus 0 (Wire) | Synth | 13 | 32E + 32EB + 4B |
| #4 | Synth Panel B2 | **0x0B** | Bus 1 (Wire1) | Synth | 13 | 32E + 32EB + 4B |
| #5 | Synth Panel C1 | **0x0C** | Bus 1 (Wire1) | Synth | 13 | 32E + 32EB + 4B |
| #6 | Synth Panel C2 | **0x0D** | Bus 1 (Wire1) | Synth | 13 | 32E + 32EB + 4B |
| #7 | Synth Panel D1 | **0x0E** | Bus 2 (Wire2) | Synth | 13 | 32E + 32EB + 4B |
| #8 | Synth Panel D2 | **0x0F** | Bus 2 (Wire2) | Synth | 13 | 32E + 32EB + 4B |
| #9 | FX Panel | **0x10** | Bus 2 (Wire2) | FX | 11 | 28E + 28EB |
| #10 | Snapshot Panel | N/A | UART (ESP10) | Snapshot | 3 | 19 Buttons |
| #11 | WiFi/Web Server | N/A | UART (ESP11) | WiFi | 0 | Web Interface |

**Legend:**
- **E** = Encoders
- **EB** = Encoder Buttons
- **B** = Standalone Buttons
- **Synth** = Standard 32-encoder synth panel configuration
- **FX** = Effects panel (28 encoders)
- **Snapshot** = Snapshot recall panel (19 buttons)

---

## I2C Bus Topology

```
                     ┌──────────────────────────────────┐
                     │  Teensy 4.0 Main Controller      │
                     │  (3 × I2C Master Interfaces)     │
                     └──────────────────────────────────┘
                          │         │         │
             ┌────────────┘         │         └────────────┐
             │                      │                      │
     ┌───────▼────────┐    ┌───────▼────────┐    ┌───────▼────────┐
     │   I2C Bus 0    │    │   I2C Bus 1    │    │   I2C Bus 2    │
     │   (Wire)       │    │   (Wire1)      │    │   (Wire2)      │
     │   1 MHz        │    │   1 MHz        │    │   1 MHz        │
     └────────────────┘    └────────────────┘    └────────────────┘
       │    │    │           │    │    │           │    │    │
     ┌─▼─┐┌─▼─┐┌─▼─┐       ┌─▼─┐┌─▼─┐┌─▼─┐       ┌─▼─┐┌─▼─┐┌─▼─┐
     │#1 ││#2 ││#3 │       │#4 ││#5 ││#6 │       │#7 ││#8 ││#9 │
     │.08││.09││.0A│       │.0B││.0C││.0D│       │.0E││.0F││.10│
     │32E││32E││32E│       │32E││32E││32E│       │32E││32E││28E│
     └───┘└───┘└───┘       └───┘└───┘└───┘       └───┘└───┘└───┘
     Synth A           Synth B           Synth C           Synth D + FX
```

**Key Features:**
- **3 parallel I2C buses** = No bus contention, maximum throughput
- **3 slaves per bus** = 38% bus utilization (well under 24 slave maximum)
- **1 MHz Fast Mode+** = Sub-500µs latency per slave
- **Individual INT lines** = ESP32s signal when events are ready

---

## Firmware Configuration Guide

### Before Flashing ESP32 Firmware

**CRITICAL:** You must set the correct I2C address in `firmware/esp32_peripheral/src/main.cpp` before uploading to each panel.

**Location:** Line 36 in `main.cpp`
```cpp
#define I2C_ADDRESS 0x08  // ← Change this value!
```

### Address Assignment by Panel

**Synth Panel #1 (A1):**
```cpp
#define I2C_ADDRESS 0x08
```

**Synth Panel #2 (A2):**
```cpp
#define I2C_ADDRESS 0x09
```

**Synth Panel #3 (B1):**
```cpp
#define I2C_ADDRESS 0x0A
```

**Synth Panel #4 (B2):**
```cpp
#define I2C_ADDRESS 0x0B
```

**Synth Panel #5 (C1):**
```cpp
#define I2C_ADDRESS 0x0C
```

**Synth Panel #6 (C2):**
```cpp
#define I2C_ADDRESS 0x0D
```

**Synth Panel #7 (D1):**
```cpp
#define I2C_ADDRESS 0x0E
```

**Synth Panel #8 (D2):**
```cpp
#define I2C_ADDRESS 0x0F
```

**FX Panel #9:**
```cpp
#define I2C_ADDRESS 0x10
#define SR_NUM_CHIPS 11      // FX panel uses 11 chips
#define NUM_ENCODERS 28      // 28 encoders
#define NUM_BUTTONS 28       // 28 encoder buttons (no standalone buttons)
```

---

## Testing Address Configuration

### Health Check via Serial Monitor

After flashing firmware with the correct address, you can verify using the Teensy serial monitor:

**Expected Output:**
```
MIDI Kraken Main Controller
Teensy 4.0 - Multi-Bus I2C Master

I2C buses initialized:
  Wire (Bus 0): 1000000 Hz
  Wire1 (Bus 1): 1000000 Hz
  Wire2 (Bus 2): 1000000 Hz

Checking ESP32 slave health:
  ESP32 #1 (0x08) [Synth Panel]: OK
  ESP32 #2 (0x09) [Synth Panel]: OK
  ESP32 #3 (0x0A) [Synth Panel]: OK
  ESP32 #4 (0x0B) [Synth Panel]: OK
  ESP32 #5 (0x0C) [Synth Panel]: OK
  ESP32 #6 (0x0D) [Synth Panel]: OK
  ESP32 #7 (0x0E) [Synth Panel]: OK
  ESP32 #8 (0x0F) [Synth Panel]: OK
  ESP32 #9 (0x10) [FX Panel]: OK
```

### Using I2C Scanner (Arduino)

Upload this sketch to each ESP32 to verify its address:

```cpp
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("I2C Scanner");
}

void loop() {
  for (byte addr = 0x08; addr <= 0x10; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Found device at 0x%02X\n", addr);
    }
  }
  delay(5000);
}
```

---

## Physical Panel Labeling

**Recommended:** Label each PCB with its I2C address during assembly to prevent confusion.

### Label Sticker Format

```
┌──────────────────────┐
│ MIDI KRAKEN         │
│ Synth Panel A1      │
│ I2C: 0x08           │
│ Bus 0 (Wire)        │
└──────────────────────┘
```

### Where to Place Labels

- **Front panel:** Small label near I2C connector
- **PCB back:** Permanent marker on soldermask
- **Enclosure:** Label on internal mounting bracket

---

## Troubleshooting

### Problem: Panel not detected by Teensy

**Check:**
1. I2C address is unique (no duplicates in system)
2. Address matches firmware configuration
3. Pull-up resistors installed (4.7kΩ on SDA/SCL)
4. I2C cable wiring correct (SDA/SCL not swapped)
5. ESP32 is powered (5V at VIN pin)

**Test:**
```cpp
// On Teensy, check if address responds
Wire.beginTransmission(0x08);  // Replace with panel address
uint8_t error = Wire.endTransmission();
if (error == 0) {
  Serial.println("Panel responds!");
} else {
  Serial.printf("Error: %d\n", error);
}
```

### Problem: Multiple panels have same address

**Symptom:** Teensy receives duplicate/corrupted events, bus errors

**Solution:**
1. Disconnect all I2C panels
2. Reconnect one panel at a time
3. Verify unique address using Teensy health check
4. If duplicate found, re-flash firmware with correct address

### Problem: Panel on wrong bus

**Symptom:** Panel detected but slow response or errors

**Check bus assignment:**
- Addresses 0x08-0x0A must connect to Teensy **Wire** (Bus 0)
- Addresses 0x0B-0x0D must connect to Teensy **Wire1** (Bus 1)
- Addresses 0x0E-0x10 must connect to Teensy **Wire2** (Bus 2)

**Teensy Pin Reference:**
- **Bus 0 (Wire):** SDA=18, SCL=19
- **Bus 1 (Wire1):** SDA=17, SCL=16
- **Bus 2 (Wire2):** SDA=25, SCL=24

---

## Address Reservation and Expansion

**Current Usage:** 9 I2C addresses (0x08-0x10)

**Available for Future Expansion:**
- 0x11-0x77 (reserved for future panels)
- Maximum per bus: 24 devices
- System maximum: 72 devices (3 buses × 24)

**If adding new panels:**
1. Assign next available address sequentially
2. Distribute across buses (maintain 3 per bus if possible)
3. Update `MultiI2CMaster.h` NUM_SLAVES constant
4. Update `MultiI2CMaster.cpp` initSlaves() function
5. Update `Protocol.h` SystemStatus arrays

---

## Summary

**8 Synth Panels:** Addresses **0x08 - 0x0F**
**1 FX Panel:** Address **0x10**
**Snapshot/WiFi:** Not on I2C (separate interfaces)

**Before flashing each panel:**
1. Open `firmware/esp32_peripheral/src/main.cpp`
2. Change `#define I2C_ADDRESS` to match panel position
3. Upload firmware
4. Label panel with address
5. Test with Teensy health check

**Bus assignment is critical:**
- Panels 1-3 → Bus 0 (Wire)
- Panels 4-6 → Bus 1 (Wire1)
- Panels 7-9 → Bus 2 (Wire2)

Following this guide ensures proper I2C addressing and prevents communication conflicts.
