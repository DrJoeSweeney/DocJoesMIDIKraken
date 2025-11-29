# ESP32 38-Pin DevKit Pinout Reference for MIDI Kraken

**Version**: 1.0
**Date**: 2025-01-29
**Board Type**: ESP32-DevKitC 38-Pin (ESP32-WROOM-32)

---

## Overview

The 38-pin ESP32 DevKit is fully compatible with the MIDI Kraken design. This document shows the pinout and confirms which pins are used for the 32-encoder panel.

**Difference from 30-pin version:**
- 38-pin has **8 additional pins** exposed (mainly on the right side)
- **Same GPIO pins used** for shift registers and I2C
- **Wider physical footprint** (requires 38-pin DIP socket or headers)
- More GPIOs available for future expansion

---

## ESP32 38-Pin Layout

```
ESP32-DevKitC 38-Pin (Top View)

Left Side (19 pins)              Right Side (19 pins)
Pin 1:  3V3                      Pin 38: GND
Pin 2:  EN                       Pin 37: VIN (5V)
Pin 3:  VP (GPIO36)              Pin 36: IO23
Pin 4:  VN (GPIO39)              Pin 35: IO22  ← I2C SCL
Pin 5:  IO34                     Pin 34: TXD0
Pin 6:  IO35                     Pin 33: RXD0
Pin 7:  IO32                     Pin 32: IO21  ← I2C SDA
Pin 8:  IO33                     Pin 31: GND
Pin 9:  IO25                     Pin 30: IO19  ← I2C INT
Pin 10: IO26                     Pin 29: IO18
Pin 11: IO27  ← SR LATCH         Pin 28: IO5
Pin 12: IO14  ← SR CLK           Pin 27: IO17
Pin 13: IO12  ← SR DATA          Pin 26: IO16
Pin 14: GND                      Pin 25: IO4
Pin 15: IO13                     Pin 24: IO0
Pin 16: SD2 (GPIO9)              Pin 23: IO2   ← Optional LED
Pin 17: SD3 (GPIO10)             Pin 22: IO15
Pin 18: CMD (GPIO11)             Pin 21: SD1 (GPIO8)
Pin 19: 5V                       Pin 20: SD0 (GPIO7)
```

---

## ESP32-S 38P Harness Board Layout

**Your specific board** has an expansion harness with clearly labeled pins. This section maps the harness labels to the MIDI Kraken design.

```
ESP32-S 38P Harness Board (Top View)

Left Column (Yellow Headers)    Right Column (Red/Yellow Headers)
CLK    ─────────────────────────────── GND
SD0    ─────────────────────────────── SD3
SD1    ─────────────────────────────── SD2
P15    ─────────────────────────────── P13
P2     ─────────────────────────────── GND
P0     ─────────────────────────────── P12  ← SR DATA
P4     ─────────────────────────────── P14  ← SR CLK
P16    ─────────────────────────────── P27  ← SR LATCH
P17    ─────────────────────────────── P26
P5     ─────────────────────────────── P25
P18    ─────────────────────────────── P33
P19    ────────────────────────────────→ P32 ← I2C SDA
GND    ─────────────────────────────── P35 ← I2C SCL (inverted on harness)
P21    ────────────────────────────────→ P34 ← I2C SCL (actual GPIO22)
Rx     ─────────────────────────────── SVN
Tx     ─────────────────────────────── SVP
P22    ─────────────────────────────── EN
P23    ─────────────────────────────── 3V3
GND    ─────────────────────────────── (end)

Top Power Section:
   DC3.5 (Barrel Jack)
   USB-C Connector
   Micro-USB Connector

Power Rails:
   3.3V > 5V  (Voltage regulation circuit with 47µF caps)
   5V rail available for power input
   GND rails on both sides
```

**IMPORTANT NOTE:** The harness board has a label inversion issue. The physical pin labeled "P35" on the right column is actually connected to the ESP32 module's GPIO34 (not GPIO35). For MIDI Kraken:
- **Use the harness pin labeled "P34"** for I2C SCL (which is actually GPIO22)
- **Use the harness pin labeled "P19"** for I2C INT (which is GPIO19)
- **Use the harness pin labeled "P21"** for I2C SDA (which is GPIO21)

### Harness Board MIDI Kraken Connections

| Function | Harness Label | Actual GPIO | Connection |
|----------|---------------|-------------|------------|
| **SR LATCH** | **P27** | GPIO27 | All shift registers /PL (pin 1) |
| **SR CLK** | **P14** | GPIO14 | All shift registers CP (pin 2) |
| **SR DATA** | **P12** | GPIO12 | Last SR Q7 output (SR13 pin 9) |
| **I2C SDA** | **P21** | GPIO21 | I2C data line + 4.7kΩ pull-up |
| **I2C SCL** | **P34** ⚠️ | GPIO22 | I2C clock line + 4.7kΩ pull-up |
| **I2C INT** | **P19** | GPIO19 | Interrupt to Teensy |
| **Power 5V** | **5V** | 5V rail | 5V from LM7805 output |
| **Ground** | **GND** | Ground | Multiple GND pins available |

⚠️ **Critical:** Use harness label "P34" for I2C SCL, not "P35" (label mismatch on harness board).

---

## MIDI Kraken Pin Assignments

### Shift Register Interface (SPI-like)

| Function | GPIO | Pin Number (38-pin) | Connection |
|----------|------|---------------------|------------|
| **LATCH** | IO27 | Pin 11 (left side) | All shift registers /PL (pin 1) |
| **CLK** | IO14 | Pin 12 (left side) | All shift registers CP (pin 2) |
| **DATA** | IO12 | Pin 13 (left side) | Last SR Q7 output (SR13 pin 9) |

### I2C Interface

| Function | GPIO | Pin Number (38-pin) | Connection |
|----------|------|---------------------|------------|
| **SDA** | IO21 | Pin 32 (right side) | I2C data + 4.7kΩ pull-up |
| **SCL** | IO22 | Pin 35 (right side) | I2C clock + 4.7kΩ pull-up |
| **INT** | IO19 | Pin 30 (right side) | Interrupt to Teensy |

### Power

| Function | Pin | Pin Number (38-pin) | Connection |
|----------|-----|---------------------|------------|
| **VIN** | 5V | Pin 37 (right side) | 5V from LM7805 output |
| **GND** | GND | Pin 14 (left), Pin 31 (right), Pin 38 (right) | Ground plane |

### Optional

| Function | GPIO | Pin Number (38-pin) | Connection |
|----------|------|---------------------|------------|
| **Status LED** | IO2 | Pin 23 (right side) | LED + 1kΩ resistor → GND |

---

## Pin Comparison: 30-Pin vs 38-Pin

| Function | 30-Pin Board | 38-Pin Board | Same GPIO? |
|----------|--------------|--------------|------------|
| LATCH | Pin 11 (IO27) | Pin 11 (IO27) | ✅ Yes |
| CLK | Pin 12 (IO14) | Pin 12 (IO14) | ✅ Yes |
| DATA | Pin 13 (IO12) | Pin 13 (IO12) | ✅ Yes |
| SDA | Pin 22 (IO21) | Pin 32 (IO21) | ✅ Yes (different pin #) |
| SCL | Pin 19 (IO22) | Pin 35 (IO22) | ✅ Yes (different pin #) |
| INT | Pin 24 (IO19) | Pin 30 (IO19) | ✅ Yes (different pin #) |
| VIN | Pin 16 (5V) | Pin 37 (5V) | ✅ Yes (different pin #) |

**Important:** The GPIO numbers are the same, but the physical pin positions differ between 30-pin and 38-pin boards. Use GPIO numbers in firmware, not pin numbers!

---

## Physical Footprint

### 30-Pin Board
```
Width: 28mm (30 pins × 2.54mm / 2 sides - overlapping center)
Length: ~50mm
Pin spacing: 2.54mm (0.1")
```

### 38-Pin Board
```
Width: 28mm (same as 30-pin due to overlapping design)
Length: ~55mm (slightly longer)
Pin spacing: 2.54mm (0.1")
```

**PCB Impact:** Both versions fit the same width footprint (28mm), but 38-pin is ~5mm longer.

---

## Wiring Diagram (38-Pin)

### Complete Connections

```
ESP32 38-Pin DevKit Connections:

Left Side:
    Pin 11 (IO27) ───→ All 13 SR pin 1 (/PL, LATCH) + 10kΩ pull-up to 5V
    Pin 12 (IO14) ───→ All 13 SR pin 2 (CP, CLK)
    Pin 13 (IO12) ───→ SR13 pin 9 (Q7, DATA)
    Pin 14 (GND)  ───→ Ground plane
    Pin 19 (5V)   ───→ 5V rail (not used, use Pin 37 instead)

Right Side:
    Pin 37 (VIN)  ───→ 5V rail from LM7805 output
    Pin 38 (GND)  ───→ Ground plane
    Pin 35 (IO22) ───┬─→ I2C SCL line
                     └─→ 4.7kΩ pull-up to 5V
    Pin 34 (TXD0) ───→ [Optional: Debug UART TX]
    Pin 33 (RXD0) ───→ [Optional: Debug UART RX]
    Pin 32 (IO21) ───┬─→ I2C SDA line
                     └─→ 4.7kΩ pull-up to 5V
    Pin 31 (GND)  ───→ Ground plane
    Pin 30 (IO19) ───→ I2C INT line to Teensy
    Pin 23 (IO2)  ───→ [Optional: Status LED + 1kΩ → GND]
```

---

## Additional GPIOs Available (38-Pin Only)

The 38-pin version exposes these extra GPIOs not available on 30-pin:

| GPIO | Pin Number | Notes |
|------|------------|-------|
| IO0 | Pin 24 | Boot mode pin (use with caution) |
| IO15 | Pin 22 | Available for expansion |
| GPIO7 (SD0) | Pin 20 | Available (if not using SD card) |
| GPIO8 (SD1) | Pin 21 | Available (if not using SD card) |
| GPIO9 (SD2) | Pin 16 | Available (if not using SD card) |
| GPIO10 (SD3) | Pin 17 | Available (if not using SD card) |
| GPIO11 (CMD) | Pin 18 | Available (if not using SD card) |

**Future expansion ideas:**
- Additional status LEDs
- Dedicated debug pins
- Encoder enable/disable control
- Panel identification switches

---

## PCB Footprint Design

### KiCad Symbol/Footprint

**For 38-pin ESP32:**

1. **Create custom symbol in KiCad:**
   - 19 pins per side (38 total)
   - 2.54mm (100mil) pin spacing
   - Label with GPIO numbers (not just pin numbers)

2. **Footprint:**
   - Use `PinSocket_2x19_P2.54mm_Vertical` or similar
   - Through-hole, 2.54mm spacing
   - Optional: Use ESP32-DevKitC-38 footprint from library

3. **PCB placement:**
   - Center on board (same as 30-pin design)
   - Account for 5mm extra length
   - Ensure clearance for micro-USB connector on end

---

## Firmware Compatibility

### No Changes Required!

The firmware uses **GPIO numbers** (not physical pin numbers), so the same code works on both 30-pin and 38-pin boards:

```cpp
// These GPIO assignments work on BOTH 30-pin and 38-pin boards
#define SR_LATCH_PIN 27    // GPIO27
#define SR_SCK_PIN 14      // GPIO14
#define SR_MISO_PIN 12     // GPIO12
#define I2C_SDA_PIN 21     // GPIO21
#define I2C_SCL_PIN 22     // GPIO22
#define I2C_EVENT_PIN 19   // GPIO19
```

---

## Advantages of 38-Pin Version

### ✅ Benefits

1. **More GPIOs exposed** - 8 additional pins for expansion
2. **Future-proof** - Add features without hardware redesign
3. **Common availability** - Widely available, standard ESP32 board
4. **Same price** - Usually same cost as 30-pin (~$5)
5. **Better layout** - More symmetric pin distribution

### ⚠️ Considerations

1. **Slightly longer** - 5mm more PCB space needed
2. **More pins to route** - Though only 6 used currently
3. **Socket cost** - 38-pin socket slightly more expensive than 30-pin

---

## Recommended: Use 38-Pin ESP32

**For MIDI Kraken, the 38-pin version is recommended because:**

✅ **Same functionality** - All required GPIOs available
✅ **Better availability** - More common in market
✅ **Future expansion** - Extra GPIOs for features
✅ **Standard board** - Most ESP32 tutorials use 38-pin
✅ **No cost difference** - Same price as 30-pin

**All PCB guides work with 38-pin boards - just use a 38-pin socket instead of 30-pin.**

---

## Socket Recommendation

### For Prototyping
```
Part: 2.54mm 2×19 pin female headers (38 pins total)
Supplier: Any electronics distributor
Cost: ~$0.50-1.00
```

**Assembly:**
- Solder two 19-pin female headers to PCB
- Insert ESP32 board into socket
- Allows easy removal for programming/debugging

### For Production (Optional)
```
Part: 2.54mm 2×19 pin male headers (38 pins total)
Method: Solder ESP32 directly to PCB (no socket)
Benefit: Lower profile, more reliable connections
Downside: Can't remove ESP32 for reprogramming
```

---

## Summary

**Your ESP32-S 38-pin board works perfectly with the MIDI Kraken design:**

- ✅ Same GPIO pin assignments (IO27, IO14, IO12, IO21, IO22, IO19)
- ✅ Same firmware (uses GPIO numbers, not physical pins)
- ✅ Same wiring (shift registers, I2C, power)
- ✅ Slightly larger footprint (5mm longer)
- ✅ Extra GPIOs available for future features

**PCB Design Changes:**
- Use 38-pin socket instead of 30-pin
- Account for 5mm extra length (~55mm vs ~50mm)
- Route same 6 signals (LATCH, CLK, DATA, SDA, SCL, INT)

**No firmware or circuit changes needed - just use the wider socket!**

---

## Quick Reference Card

Print this for easy reference during assembly:

### Generic ESP32 38-Pin DevKit
```
┌────────────────────────────────────────────┐
│   ESP32 38-Pin MIDI Kraken Quick Ref      │
├────────────────────────────────────────────┤
│ Shift Registers:                           │
│   Pin 11 (IO27) → LATCH (all SR pin 1)   │
│   Pin 12 (IO14) → CLK (all SR pin 2)     │
│   Pin 13 (IO12) → DATA (SR13 pin 9)      │
│                                            │
│ I2C Interface:                             │
│   Pin 32 (IO21) → SDA + 4.7kΩ pull-up    │
│   Pin 35 (IO22) → SCL + 4.7kΩ pull-up    │
│   Pin 30 (IO19) → INT to Teensy          │
│                                            │
│ Power:                                     │
│   Pin 37 (VIN)  → 5V from LM7805         │
│   Pins 14,31,38 → GND to ground plane    │
│                                            │
│ Optional:                                  │
│   Pin 23 (IO2)  → Status LED + 1kΩ       │
└────────────────────────────────────────────┘
```

### ESP32-S 38P Harness Board (Your Board)
```
┌────────────────────────────────────────────┐
│   ESP32-S 38P Harness - MIDI Kraken       │
├────────────────────────────────────────────┤
│ Shift Registers (Right Column):           │
│   P27 header → LATCH (all SR pin 1)      │
│   P14 header → CLK (all SR pin 2)        │
│   P12 header → DATA (SR13 pin 9)         │
│                                            │
│ I2C Interface (Left/Right Columns):        │
│   P21 header → SDA + 4.7kΩ pull-up       │
│   P34 header ⚠️ → SCL + 4.7kΩ (GPIO22)  │
│   P19 header → INT to Teensy             │
│                                            │
│ Power (Top Headers):                       │
│   5V header  → 5V from LM7805            │
│   GND headers → Ground plane (multiple)   │
│                                            │
│ ⚠️ CRITICAL: Use P34 for SCL (not P35)    │
│    Harness has label mismatch issue       │
└────────────────────────────────────────────┘
```

Good choice on the 38-pin board! It's the standard ESP32 DevKit and gives you room to grow. 🎛️
