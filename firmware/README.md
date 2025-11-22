# MIDI Kraken Firmware

This directory contains all firmware and libraries for the MIDI Kraken project.

## 📁 Directory Structure

```
firmware/
├── libraries/                    # Shared libraries (24 libraries)
│   ├── Protocol/                # Data structures and constants
│   ├── ShiftRegister/           # 74HC165 bit-banging reader
│   ├── ShiftRegisterDMA/        # DMA-accelerated reader (ESP32)
│   ├── EncoderDecoder/          # Quadrature decoder
│   ├── ButtonHandler/           # Debounced button handler
│   ├── LockFreeQueue/           # Inter-core queue (ESP32)
│   ├── I2CSlave/                # I2C slave (ESP32)
│   ├── I2CMaster/               # Simple I2C master
│   ├── MultiI2CMaster/          # 3-bus I2C master (Teensy)
│   ├── StateManager/            # 619-control state manager
│   ├── MIDIEngine/              # MIDI message generation
│   ├── Joystick/                # Joystick handler
│   ├── Diagnostics/             # Performance monitoring
│   └── ...                      # Additional libraries
│
├── esp32_peripheral/            # ESP32 #1-6 firmware
│   ├── platformio.ini           # PlatformIO configuration
│   └── src/
│       └── main.cpp             # Peripheral node firmware
│
├── esp32_wifi/                  # ESP32 #7 firmware
│   ├── platformio.ini           # PlatformIO configuration
│   ├── data/                    # Web interface files
│   └── src/
│       └── main.cpp             # WiFi node firmware
│
└── teensy/                      # Teensy 4.0 firmware
    ├── platformio.ini           # PlatformIO configuration
    └── src/
        └── main.cpp             # Main controller firmware
```

## 🔧 Prerequisites

### Software
- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- [Git](https://git-scm.com/) (optional, for version control)

### Hardware
- 7 × ESP32 development boards (ESP32-DevKitC or similar)
- 1 × Teensy 4.0
- USB cables for programming

## 🚀 Quick Start

### 1. Install PlatformIO

**VS Code Extension (Recommended):**
1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install PlatformIO IDE extension from VS Code marketplace
3. Restart VS Code

**CLI:**
```bash
pip install platformio
```

### 2. Build Firmware

#### ESP32 Peripheral Node (ESP32 #1-6)

```bash
cd firmware/esp32_peripheral
pio run
```

**Before uploading to each node:**
- Edit `src/main.cpp`
- Change `I2C_ADDRESS` to match node number:
  - ESP32 #1: `0x08`
  - ESP32 #2: `0x09`
  - ESP32 #3: `0x0A`
  - ESP32 #4: `0x0B`
  - ESP32 #5: `0x0C`
  - ESP32 #6: `0x0D`
- Adjust `SR_NUM_CHIPS`, `NUM_ENCODERS`, `NUM_BUTTONS` per node

#### ESP32 WiFi Node (ESP32 #7)

```bash
cd firmware/esp32_wifi
pio run
```

WiFi credentials can be changed in `src/main.cpp`:
- `WIFI_SSID`: "MIDI_Kraken" (default)
- `WIFI_PASSWORD`: "midicontrol" (default)

#### Teensy 4.0 Main Controller

```bash
cd firmware/teensy
pio run
```

### 3. Upload Firmware

#### Using PlatformIO

**VS Code:**
1. Open firmware folder in VS Code
2. Click PlatformIO icon in sidebar
3. Select "Upload" under appropriate environment

**CLI:**
```bash
# ESP32 peripheral
cd firmware/esp32_peripheral
pio run --target upload

# ESP32 WiFi
cd firmware/esp32_wifi
pio run --target upload

# Teensy
cd firmware/teensy
pio run --target upload
```

### 4. Monitor Serial Output

```bash
# ESP32
pio device monitor --baud 115200

# Teensy
pio device monitor --baud 115200
```

Or use VS Code PlatformIO "Monitor" button.

## 📚 Library Documentation

### Protocol
Defines all shared data structures:
- `ControlConfig` (48 bytes) - Control configuration
- `ControlState` (4 bytes) - Runtime state
- `EventMessage` (8 bytes) - I2C event messages
- `Snapshot` (2488 bytes) - Snapshot data
- `SessionFile` (103KB) - Complete session
- All enums and constants

### Hardware Abstraction

**ShiftRegister**
- Standard 74HC165 bit-banging reader
- ~10µs per byte on ESP32 @ 240MHz
- Cross-platform (ESP32, Teensy, Arduino)

**ShiftRegisterDMA** (ESP32 only)
- DMA-accelerated SPI reading
- ~5µs for 16 bytes (128 bits)
- Zero CPU overhead during transfer

**EncoderDecoder**
- Gray code state machine
- Acceleration curves
- Position tracking

**ButtonHandler**
- 8-sample debouncing
- Press, release, hold detection
- Active-low/high configuration

**LockFreeQueue** (ESP32 only)
- Thread-safe SPSC queue
- Atomic operations (no locks)
- For dual-core communication

### Communication

**I2CSlave** (ESP32)
- Interrupt-driven I2C slave
- Event queue (up to 6 events/transaction)
- Command-response protocol

**I2CMaster**
- Simple single-bus master
- Sequential slave polling
- Event aggregation

**MultiI2CMaster** (Teensy)
- 3 parallel I2C buses
- Interrupt-driven polling
- Health monitoring

### State Management

**StateManager**
- Manages 619 control states
- O(1) access by globalID
- Bank A/B switching
- Dirty flag tracking

### MIDI

**MIDIEngine**
- 7-bit and 14-bit MIDI
- 4 virtual USB devices
- Pitch bend, CC, Program Change
- Message throttling (2,500 msg/sec)

### I/O

**Joystick**
- 2-axis analog input
- Dead zone and calibration
- Pitch bend (X) + Modulation (Y)

**Diagnostics**
- Performance metrics
- Latency tracking
- Drop rate monitoring

## 🔌 Hardware Connections

### ESP32 Peripheral Nodes (#1-6)

```
Shift Registers:
  MISO (Q7) → GPIO 12
  SCK (CLK) → GPIO 14
  Latch (SH/LD) → GPIO 27

I2C:
  SDA → GPIO 21
  SCL → GPIO 22
  EVENT → GPIO 19 (output to Teensy)
```

### ESP32 WiFi Node (#7)

```
Shift Registers: Same as peripheral nodes

I2C: Same as peripheral nodes

SD Card:
  MOSI → GPIO 23
  MISO → GPIO 19
  SCK → GPIO 18
  CS → GPIO 5
```

### Teensy 4.0

```
I2C Bus 0 (Wire):
  SDA → Pin 18 (ESP32 #1, #2)
  SCL → Pin 19

I2C Bus 1 (Wire1):
  SDA → Pin 17 (ESP32 #3, #4)
  SCL → Pin 16

I2C Bus 2 (Wire2):
  SDA → Pin 25 (ESP32 #5, #6, #7)
  SCL → Pin 24

Joystick:
  X-axis → A0
  Y-axis → A1
  Button → Pin 2

Event Interrupt:
  INT → Pin 3 (from ESP32s)

LED:
  Status LED → Pin 13
```

## 🧪 Testing

### Unit Testing Individual Libraries

Each library can be tested independently:

```cpp
#include <ShiftRegister.h>

ShiftRegister sr(12, 14, 27, 16);  // data, clock, latch, numChips

void setup() {
    Serial.begin(115200);
    sr.begin();
}

void loop() {
    sr.read();
    uint8_t byte0 = sr.getByte(0);
    Serial.println(byte0, HEX);
    delay(100);
}
```

### Prototype Testing (8 Encoders)

See **[NEXT STEPS.md](../NEXT%20STEPS.md)** Phase 3 for prototype build guide.

### System Integration Testing

1. **ESP32 Health Check:**
   - Upload firmware to all 7 ESP32s
   - Check serial output for successful initialization
   - Verify I2C addresses are unique

2. **Teensy Health Check:**
   - Upload firmware to Teensy
   - Check serial output for ESP32 slave detection
   - Should show 7 healthy slaves

3. **Event Flow Test:**
   - Turn encoders on ESP32 #1
   - Monitor Teensy serial output
   - Should see events received and MIDI sent

4. **WiFi Configuration:**
   - Connect to "MIDI_Kraken" WiFi network
   - Browse to http://192.168.4.1
   - Verify web interface loads

## 🐛 Troubleshooting

### ESP32 Not Responding
- Check power supply (5V 3A minimum)
- Verify I2C address is unique
- Check shift register connections
- Monitor serial output for errors

### Teensy Not Receiving Events
- Verify I2C connections (SDA, SCL, GND)
- Check I2C pull-up resistors (4.7kΩ recommended)
- Confirm ESP32s are on correct buses
- Monitor with logic analyzer if needed

### MIDI Not Working
- Verify USB MIDI device appears in DAW
- Check Teensy USB type is set to "MIDI" in platformio.ini
- Test with MIDI monitor software
- Ensure controls are enabled in configuration

### SD Card Not Detected
- Check SD card formatting (FAT32)
- Verify SPI connections
- Try different SD card (some are incompatible)
- Check CS pin definition

### Low Scan Rate
- Reduce shift register count
- Increase SPI clock speed
- Optimize Core 0 task
- Check for blocking operations

## 📖 Additional Resources

- **[Complete Software Specification](../MIDI%20Kraken%20-%20Complete%20Software%20Specification.md)** - Full technical details
- **[CHANGELOG](../CHANGELOG.md)** - Project evolution and design decisions
- **[NEXT STEPS](../NEXT%20STEPS.md)** - Implementation roadmap

## 🔄 Development Workflow

### Adding a New Feature

1. **Define in Protocol.h** if it needs new data structures
2. **Create or modify library** in `libraries/`
3. **Update firmware** in appropriate target (ESP32/Teensy)
4. **Test incrementally** on hardware
5. **Document changes** in code comments

### Example: Adding MIDI Learn

1. Add `MIDI_LEARN` mode to `Protocol.h`
2. Extend `StateManager` to track learn state
3. Update Teensy firmware to listen for incoming MIDI
4. Add web interface button in ESP32 WiFi firmware
5. Test with MIDI controller

## 📄 License

[Your License Here]

## 🤝 Contributing

This is a personal project, but feedback and suggestions are welcome!

---

**Version:** 1.0.0
**Last Updated:** 2025-01-22
**Status:** Core libraries complete, ready for hardware testing
