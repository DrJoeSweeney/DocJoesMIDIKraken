# PCB Builder's Guide: 32-Encoder Panel for MIDI Kraken

**Version**: 1.0
**Date**: 2025-01-29
**Design Tool**: KiCad 9
**Panel Type**: Synth Panel (32 encoders + 32 encoder buttons + 4 standalone buttons)
**Board Layers**: 2-layer (Top + Bottom)

---

## Table of Contents

1. [Overview](#overview)
2. [Complete Bill of Materials](#complete-bill-of-materials)
3. [Power Distribution Architecture](#power-distribution-architecture)
4. [Component Placement Strategy](#component-placement-strategy)
5. [ESP32 DevKit Connections](#esp32-devkit-connections)
6. [Shift Register Chain](#shift-register-chain)
7. [Encoder Wiring](#encoder-wiring)
8. [Button Wiring](#button-wiring)
9. [I2C Interface](#i2c-interface)
10. [Voltage Regulation](#voltage-regulation)
11. [Decoupling and Filtering](#decoupling-and-filtering)
12. [PCB Layout Guidelines](#pcb-layout-guidelines)
13. [Testing and Troubleshooting](#testing-and-troubleshooting)

---

## Overview

This guide describes the complete electrical design for a single 32-encoder panel. Eight of these panels (plus FX and snapshot panels) comprise the complete MIDI Kraken controller.

**Panel Specifications:**
- **32 × KY-050 rotary encoders** (quadrature, with integrated push button)
- **4 × tactile push buttons** (standalone buttons)
- **13 × 74HC165 shift registers** (8-bit PISO, DIP-16 package)
- **1 × ESP32 DevKit** (30-pin or 38-pin DIP-compatible dev board)
- **1 × LM7805 voltage regulator** (5V, TO-220 package)
- **Total I/O bits:** 100 (64 encoder signals + 32 encoder buttons + 4 standalone buttons)

**Signal Flow:**
```
Encoders/Buttons → Shift Registers (Parallel Load) → ESP32 (SPI Read) → Teensy (I2C)
```

**Physical Dimensions:**
- Estimated PCB size: 300mm × 120mm (adjust based on encoder spacing)
- Mounting holes: 4 × M3 at corners
- Encoder grid: 8 × 4 or 4 × 8 (adjust to panel layout preference)

---

## Complete Bill of Materials

### ICs and Active Components

| Qty | Part Number | Description | Package | Notes |
|-----|-------------|-------------|---------|-------|
| 1 | ESP32-DevKitC | ESP32 development board | 30-pin DIP | NodeMCU-32S or similar |
| 13 | 74HC165N | 8-bit PISO shift register | DIP-16 | Must be HC (not HCT) |
| 1 | LM7805 | 5V voltage regulator | TO-220 | 1A minimum |
| 1 | 1N4001 | Rectifier diode (reverse polarity protection) | DO-41 | 1A, 50V |

### Encoders and Buttons

| Qty | Part Number | Description | Mounting | Notes |
|-----|-------------|-------------|----------|-------|
| 32 | KY-050 | Rotary encoder with switch | Panel mount | 3 pins (CLK, DT, SW) |
| 4 | Tactile switch | 6mm tactile button | Through-hole | SPST, normally open |

### Resistors (1/4W, 5% tolerance)

| Qty | Value | Purpose | Location |
|-----|-------|---------|----------|
| 2 | 4.7kΩ | I2C pull-up resistors | SDA, SCL lines |
| 13 | 10kΩ | Shift register /PL pull-up | SR pin 1 (/PL) |
| 36 | 10kΩ | Encoder/button pull-up resistors | All switch inputs |
| 1 | 1kΩ | Status LED current limiting | LED circuit |

**Total resistors:** 52

### Capacitors

| Qty | Value | Type | Voltage | Purpose |
|-----|-------|------|---------|---------|
| 13 | 100nF (0.1µF) | Ceramic | 50V | 74HC165 decoupling (one per chip) |
| 1 | 100nF (0.1µF) | Ceramic | 50V | ESP32 VCC decoupling |
| 2 | 10µF | Electrolytic | 16V | LM7805 input/output filtering |
| 1 | 470µF | Electrolytic | 16V | Bulk power supply filter |
| 36 | 100nF (0.1µF) | Ceramic | 50V | Encoder/button debouncing (optional) |

**Total capacitors:** 53 (17 required, 36 optional)

### Connectors

| Qty | Type | Pins | Purpose |
|-----|------|------|---------|
| 1 | Barrel jack | 2.1mm | 5V power input |
| 1 | Pin header | 5-pin (2.54mm) | I2C output (SDA, SCL, INT, GND, 5V) |
| 32 | Pin header | 3-pin (2.54mm) | Encoder connections (CLK, DT, SW) |
| 4 | Pin header | 2-pin (2.54mm) | Standalone button connections |

### Miscellaneous

| Qty | Part | Purpose |
|-----|------|---------|
| 1 | LED (red or green) | Power indicator / status |
| 4 | M3 × 10mm standoff | PCB mounting |
| 1 | Heatsink (optional) | LM7805 cooling |

---

## Power Distribution Architecture

### Power Input and Regulation

**Input:** 5V DC via barrel jack (center positive)

**Power Tree:**
```
[5V Barrel Jack] → [1N4001 Diode (reverse protection)] → [470µF Cap] → [LM7805 Input]
                                                             ↓
                                                        [10µF Cap]
                                                             ↓
                                                        [LM7805 Output] → [10µF Cap] → [5V Rail]
                                                                                            ↓
                                                                    ┌───────────────────────┴─────────────────────────┐
                                                                    ↓                       ↓                         ↓
                                                            [ESP32 5V]            [13 × 74HC165 VCC]         [Pull-up resistors]
```

**LM7805 Pin Connections:**
```
     LM7805 (TO-220, front view)
     ┌─────────────┐
     │   1  2  3   │
     └──┬──┬──┬───┘
        │  │  │
       IN GND OUT
        │  │  │
        └──┴──┘
```

**Pin Assignments:**
- **Pin 1 (IN):** 5V from barrel jack (after diode and 470µF cap)
- **Pin 2 (GND):** Common ground
- **Pin 3 (OUT):** Regulated 5V to ESP32 and shift registers

**Power Budget:**
- ESP32: ~240mA peak
- 13 × 74HC165: ~13mA (1mA each)
- Pull-up resistors: ~10mA
- **Total:** ~263mA per panel (well within LM7805 1A limit)

### Ground Plane

- **Bottom layer:** Solid ground plane (maximize copper pour)
- **Top layer:** Ground fill in unused areas
- **Connect:** All ground pins to plane via short traces or direct vias
- **Star ground:** Connect all major ground points (ESP32, shift registers, power input) to a single central point if possible

---

## Component Placement Strategy

### Recommended Layout (Top View)

```
┌──────────────────────────────────────────────────────────────────────┐
│  POWER SECTION                 SHIFT REGISTER CHAIN                  │
│  ┌─────────┐                   ┌───┬───┬───┬───┬───┬───┬───┐        │
│  │Barrel   │  ┌──┐             │SR1│SR2│SR3│SR4│SR5│SR6│SR7│        │
│  │Jack     │  │  │ LM7805      └───┴───┴───┴───┴───┴───┴───┘        │
│  └─────────┘  └──┘             ┌───┬───┬───┬───┬───┬───┐            │
│                                 │SR8│SR9│S10│S11│S12│S13│            │
│  I2C CONNECTOR                  └───┴───┴───┴───┴───┴───┘            │
│  ┌───────┐                                                            │
│  │5-pin  │                                                            │
│  │header │     ESP32 DEVKIT                                          │
│  └───────┘     ┌─────────────────────────┐                          │
│                │ [===================]   │                          │
│                │ ESP32-DevKitC           │                          │
│                │ 30-pin DIP socket       │                          │
│                └─────────────────────────┘                          │
│                                                                       │
│  ENCODER GRID (8 × 4 or 4 × 8 layout)                               │
│  ┌───┬───┬───┬───┬───┬───┬───┬───┐                                 │
│  │E1 │E2 │E3 │E4 │E5 │E6 │E7 │E8 │                                 │
│  ├───┼───┼───┼───┼───┼───┼───┼───┤                                 │
│  │E9 │E10│E11│E12│E13│E14│E15│E16│                                 │
│  ├───┼───┼───┼───┼───┼───┼───┼───┤                                 │
│  │E17│E18│E19│E20│E21│E22│E23│E24│                                 │
│  ├───┼───┼───┼───┼───┼───┼───┼───┤                                 │
│  │E25│E26│E27│E28│E29│E30│E31│E32│                                 │
│  └───┴───┴───┴───┴───┴───┴───┴───┘                                 │
│                                                                       │
│  STANDALONE BUTTONS                                                  │
│  ┌───┬───┬───┬───┐                                                  │
│  │B1 │B2 │B3 │B4 │                                                  │
│  └───┴───┴───┴───┘                                                  │
│                                                                       │
│  ○  Mounting Hole (M3)                                          ○   │
└──────────────────────────────────────────────────────────────────────┘
```

**Placement Priorities:**
1. **Power section (left side):** Barrel jack → Diode → Regulator → Caps
2. **ESP32 (center-left):** Easy access to shift registers and I2C connector
3. **Shift registers (top-center):** Linear chain arrangement
4. **Encoders (center/right):** Grid layout matching desired physical arrangement
5. **I2C connector (left edge):** Near ESP32 for short traces
6. **Standalone buttons (bottom):** Separate from encoder grid

---

## ESP32 DevKit Connections

### ESP32 DevKitC 30-Pin Pinout

**Left Side (Top to Bottom):**
```
Pin 1:  3V3        ← Do not use (use 5V pin instead)
Pin 2:  EN         ← Not connected
Pin 3:  VP (36)    ← Not connected
Pin 4:  VN (39)    ← Not connected
Pin 5:  IO34       ← Not connected
Pin 6:  IO35       ← Not connected
Pin 7:  IO32       ← Not connected
Pin 8:  IO33       ← Not connected
Pin 9:  IO25       ← Not connected
Pin 10: IO26       ← Not connected
Pin 11: IO27       ← Shift Register LATCH (SR pin 1, all chips)
Pin 12: IO14       ← Shift Register CLK (SR pin 2, all chips)
Pin 13: IO12       ← Shift Register DATA (SR pin 9, last chip only)
Pin 14: GND        ← Connect to ground plane
Pin 15: IO13       ← Not connected
```

**Right Side (Top to Bottom):**
```
Pin 16: VIN (5V)   ← Connect to 5V rail from LM7805
Pin 17: GND        ← Connect to ground plane
Pin 18: IO23       ← Not connected
Pin 19: IO22       ← I2C SCL (via 4.7kΩ pull-up to 5V)
Pin 20: TXD0       ← Not connected (or debug UART)
Pin 21: RXD0       ← Not connected (or debug UART)
Pin 22: IO21       ← I2C SDA (via 4.7kΩ pull-up to 5V)
Pin 23: GND        ← Connect to ground plane
Pin 24: IO19       ← I2C INT line (to Teensy)
Pin 25: IO18       ← Not connected
Pin 26: IO5        ← Not connected
Pin 27: IO17       ← Not connected
Pin 28: IO16       ← Not connected
Pin 29: IO4        ← Not connected
Pin 30: IO2        ← Optional: Status LED via 1kΩ resistor
```

### Critical ESP32 Connections

**Power:**
```
ESP32 Pin 16 (VIN) ──→ 5V Rail (LM7805 output)
ESP32 Pin 14 (GND) ──→ Ground Plane
ESP32 Pin 17 (GND) ──→ Ground Plane
ESP32 Pin 23 (GND) ──→ Ground Plane
```

**Shift Register Interface (SPI-like):**
```
ESP32 Pin 11 (IO27, LATCH) ──→ All 13 × SR pin 1 (/PL)
ESP32 Pin 12 (IO14, CLK)   ──→ All 13 × SR pin 2 (CP)
ESP32 Pin 13 (IO12, DATA)  ──→ SR13 pin 9 (Q7, last chip in chain)
```

**I2C Interface:**
```
ESP32 Pin 22 (IO21, SDA) ──→ I2C Connector Pin 1 (via 4.7kΩ pull-up)
ESP32 Pin 19 (IO22, SCL) ──→ I2C Connector Pin 2 (via 4.7kΩ pull-up)
ESP32 Pin 24 (IO19, INT) ──→ I2C Connector Pin 3 (interrupt to Teensy)
```

**Optional Status LED:**
```
ESP32 Pin 30 (IO2) ──→ [1kΩ resistor] ──→ LED Anode ──→ LED Cathode ──→ GND
```

---

## Shift Register Chain

### 74HC165 Pinout (DIP-16)

```
      74HC165N (DIP-16, top view)
      ┌───────────────┐
 /PL  │1            16│ VCC
 CP   │2            15│ CE (clock enable, active low)
 D4   │3            14│ D3
 D5   │4            13│ D2
 D6   │5            12│ D1
 D7   │6            11│ D0
 /Q7  │7            10│ DS (serial data in)
 GND  │8             9│ Q7 (serial data out)
      └───────────────┘
```

**Pin Functions:**
- **Pin 1 (/PL, Parallel Load):** Active low, latches parallel inputs
- **Pin 2 (CP, Clock Pulse):** Rising edge shifts data
- **Pins 3-6, 11-14 (D0-D7):** Parallel data inputs
- **Pin 7 (/Q7):** Inverted serial output (not used)
- **Pin 8 (GND):** Ground
- **Pin 9 (Q7):** Serial data output (to next chip's DS input)
- **Pin 10 (DS, Data Serial):** Serial data input (from previous chip's Q7)
- **Pin 15 (CE, Clock Enable):** Active low, tie to GND
- **Pin 16 (VCC):** 5V power

### Daisy Chain Wiring

**Chain Order:** SR1 → SR2 → SR3 → ... → SR13 → ESP32

**All Shift Registers (Parallel Connections):**
```
All SR pin 1 (/PL)  ──→ ESP32 IO27 (LATCH) + 10kΩ pull-up to 5V
All SR pin 2 (CP)   ──→ ESP32 IO14 (CLK)
All SR pin 15 (CE)  ──→ GND (permanently enabled)
All SR pin 16 (VCC) ──→ 5V Rail
All SR pin 8 (GND)  ──→ Ground Plane
```

**Serial Data Chain:**
```
ESP32 IO12 (DATA) ──→ SR13 pin 9 (Q7)
SR13 pin 10 (DS)  ──→ SR12 pin 9 (Q7)
SR12 pin 10 (DS)  ──→ SR11 pin 9 (Q7)
SR11 pin 10 (DS)  ──→ SR10 pin 9 (Q7)
SR10 pin 10 (DS)  ──→ SR9 pin 9 (Q7)
SR9 pin 10 (DS)   ──→ SR8 pin 9 (Q7)
SR8 pin 10 (DS)   ──→ SR7 pin 9 (Q7)
SR7 pin 10 (DS)   ──→ SR6 pin 9 (Q7)
SR6 pin 10 (DS)   ──→ SR5 pin 9 (Q7)
SR5 pin 10 (DS)   ──→ SR4 pin 9 (Q7)
SR4 pin 10 (DS)   ──→ SR3 pin 9 (Q7)
SR3 pin 10 (DS)   ──→ SR2 pin 9 (Q7)
SR2 pin 10 (DS)   ──→ SR1 pin 9 (Q7)
SR1 pin 10 (DS)   ──→ GND (or 10kΩ to GND)
```

**Critical Notes:**
- Data flows from SR1 → SR13 → ESP32 (reverse order)
- ESP32 reads Q7 of the *last* chip (SR13) in the chain
- Each chip's Q7 feeds the next chip's DS input
- /PL (pin 1) needs a 10kΩ pull-up resistor to prevent floating

### Decoupling Capacitors

**Every shift register needs a 100nF ceramic capacitor:**
```
SR1-SR13 pin 16 (VCC) ──→ 100nF cap ──→ SR pin 8 (GND)
```

**Placement:** As close as possible to each chip (within 5mm)

---

## Encoder Wiring

### KY-050 Encoder Pinout

```
KY-050 (bottom view, pins facing down)
┌─────────────┐
│      ○      │  ← Shaft
│             │
│  CLK DT SW  │
│   │  │  │   │
└───┼──┼──┼───┘
    1  2  3
```

**Pin Functions:**
- **CLK (Channel A):** Quadrature output A
- **DT (Data, Channel B):** Quadrature output B
- **SW (Switch):** Integrated push button (active low)

### Encoder-to-Shift-Register Mapping

**32 encoders require 96 signals total:**
- 32 × CLK signals (Channel A)
- 32 × DT signals (Channel B)
- 32 × SW signals (button presses)

**Bit allocation across 13 shift registers (104 bits total, 100 used):**

**SR1 (bits 0-7): Encoders 1-4**
```
SR1 pin 11 (D0) ──→ Encoder 1 CLK  + 10kΩ pull-up to 5V
SR1 pin 12 (D1) ──→ Encoder 1 DT   + 10kΩ pull-up to 5V
SR1 pin 13 (D2) ──→ Encoder 2 CLK  + 10kΩ pull-up to 5V
SR1 pin 14 (D3) ──→ Encoder 2 DT   + 10kΩ pull-up to 5V
SR1 pin 3  (D4) ──→ Encoder 3 CLK  + 10kΩ pull-up to 5V
SR1 pin 4  (D5) ──→ Encoder 3 DT   + 10kΩ pull-up to 5V
SR1 pin 5  (D6) ──→ Encoder 4 CLK  + 10kΩ pull-up to 5V
SR1 pin 6  (D7) ──→ Encoder 4 DT   + 10kΩ pull-up to 5V
```

**SR2 (bits 8-15): Encoders 5-8**
```
SR2 pin 11 (D0) ──→ Encoder 5 CLK  + 10kΩ pull-up to 5V
SR2 pin 12 (D1) ──→ Encoder 5 DT   + 10kΩ pull-up to 5V
SR2 pin 13 (D2) ──→ Encoder 6 CLK  + 10kΩ pull-up to 5V
SR2 pin 14 (D3) ──→ Encoder 6 DT   + 10kΩ pull-up to 5V
SR2 pin 3  (D4) ──→ Encoder 7 CLK  + 10kΩ pull-up to 5V
SR2 pin 4  (D5) ──→ Encoder 7 DT   + 10kΩ pull-up to 5V
SR2 pin 5  (D6) ──→ Encoder 8 CLK  + 10kΩ pull-up to 5V
SR2 pin 6  (D7) ──→ Encoder 8 DT   + 10kΩ pull-up to 5V
```

**Pattern continues for SR3-SR8 (Encoders 9-32):**
- SR3: Encoders 9-12 (CLK, DT pairs)
- SR4: Encoders 13-16 (CLK, DT pairs)
- SR5: Encoders 17-20 (CLK, DT pairs)
- SR6: Encoders 21-24 (CLK, DT pairs)
- SR7: Encoders 25-28 (CLK, DT pairs)
- SR8: Encoders 29-32 (CLK, DT pairs)

**SR9 (bits 64-71): Encoder buttons 1-8**
```
SR9 pin 11 (D0) ──→ Encoder 1 SW  + 10kΩ pull-up to 5V
SR9 pin 12 (D1) ──→ Encoder 2 SW  + 10kΩ pull-up to 5V
SR9 pin 13 (D2) ──→ Encoder 3 SW  + 10kΩ pull-up to 5V
SR9 pin 14 (D3) ──→ Encoder 4 SW  + 10kΩ pull-up to 5V
SR9 pin 3  (D4) ──→ Encoder 5 SW  + 10kΩ pull-up to 5V
SR9 pin 4  (D5) ──→ Encoder 6 SW  + 10kΩ pull-up to 5V
SR9 pin 5  (D6) ──→ Encoder 7 SW  + 10kΩ pull-up to 5V
SR9 pin 6  (D7) ──→ Encoder 8 SW  + 10kΩ pull-up to 5V
```

**SR10-SR12 (bits 72-95): Encoder buttons 9-32**
- SR10: Encoder buttons 9-16
- SR11: Encoder buttons 17-24
- SR12: Encoder buttons 25-32

**SR13 (bits 96-99): Standalone buttons 1-4**
```
SR13 pin 11 (D0) ──→ Button 1  + 10kΩ pull-up to 5V
SR13 pin 12 (D1) ──→ Button 2  + 10kΩ pull-up to 5V
SR13 pin 13 (D2) ──→ Button 3  + 10kΩ pull-up to 5V
SR13 pin 14 (D3) ──→ Button 4  + 10kΩ pull-up to 5V
SR13 pin 3  (D4) ──→ GND (unused)
SR13 pin 4  (D5) ──→ GND (unused)
SR13 pin 5  (D6) ──→ GND (unused)
SR13 pin 6  (D7) ──→ GND (unused)
```

### Encoder Common Connections

**All encoders share a common ground:**
```
Encoder 1-32 (common pin) ──→ Ground Plane
```

**Note:** KY-050 encoders are open-collector outputs (pull to GND when active), hence need pull-up resistors.

### Pull-Up Resistor Network

**Each encoder signal (CLK, DT, SW) needs a 10kΩ pull-up:**
```
Encoder X CLK ──→ SR pin Y ──┬──→ 10kΩ resistor ──→ 5V Rail
                              └──→ (optional: 100nF cap to GND for debounce)
```

**Total pull-ups needed:** 64 (encoder CLK/DT) + 32 (encoder SW) + 4 (standalone) = 100 pull-up resistors

**Alternative:** Use resistor networks (8 resistors in one SIP/DIP package) to save space

---

## Button Wiring

### Standalone Button Connections

**4 tactile buttons for panel-specific functions:**

```
         ┌───────┐
5V ──→ ──┤ Button├── ──→ SR13 pin 11-14 (with 10kΩ pull-up)
         └───────┘
                  └────→ GND
```

**Wiring (active low):**
```
Button 1 terminal 1 ──→ 5V Rail
Button 1 terminal 2 ──→ SR13 pin 11 (D0) + 10kΩ pull-up to 5V
Button 1 terminal 2 ──→ GND (when pressed)

Button 2 terminal 1 ──→ 5V Rail
Button 2 terminal 2 ──→ SR13 pin 12 (D1) + 10kΩ pull-up to 5V

Button 3 terminal 1 ──→ 5V Rail
Button 3 terminal 2 ──→ SR13 pin 13 (D2) + 10kΩ pull-up to 5V

Button 4 terminal 1 ──→ 5V Rail
Button 4 terminal 2 ──→ SR13 pin 14 (D3) + 10kΩ pull-up to 5V
```

**Alternative wiring (recommended for PCB):**
```
Button connects signal line to GND when pressed:
  SR pin ──┬──→ 10kΩ pull-up to 5V (idle = HIGH)
           └──→ Button ──→ GND (pressed = LOW)
```

### Debouncing (Optional but Recommended)

**Hardware debounce using capacitors:**
```
SR pin ──┬──→ 10kΩ pull-up to 5V
         └──→ 100nF capacitor to GND
```

**Effect:** Creates RC filter with ~1ms time constant (sufficient for mechanical switch bounce)

**Note:** Firmware also implements debouncing, so hardware caps are optional but improve reliability.

---

## I2C Interface

### I2C Connector Pinout

**5-pin header (2.54mm pitch):**
```
Pin 1: SDA    (Serial Data)
Pin 2: SCL    (Serial Clock)
Pin 3: INT    (Interrupt to Teensy, active low)
Pin 4: GND    (Ground)
Pin 5: 5V     (Power to Teensy, optional)
```

### I2C Signal Connections

**From ESP32 to Connector:**
```
ESP32 pin 22 (IO21, SDA) ──┬──→ I2C Connector Pin 1 (SDA)
                            └──→ 4.7kΩ pull-up to 5V

ESP32 pin 19 (IO22, SCL) ──┬──→ I2C Connector Pin 2 (SCL)
                            └──→ 4.7kΩ pull-up to 5V

ESP32 pin 24 (IO19, INT) ──────→ I2C Connector Pin 3 (INT)
                                  (no pull-up, driven by ESP32)

Ground Plane ──────────────────→ I2C Connector Pin 4 (GND)

5V Rail ───────────────────────→ I2C Connector Pin 5 (5V)
```

### I2C Pull-Up Resistors

**Why 4.7kΩ?**
- Standard I2C bus capacitance: ~100-400pF
- 1MHz Fast Mode+ requires strong pull-ups
- 4.7kΩ provides ~1mA pull-up current at 5V
- Suitable for short traces (<300mm)

**Placement:** Pull-up resistors should be near the ESP32 pins, not at the connector.

### I2C Trace Routing

**Best Practices:**
- Keep SDA and SCL traces parallel and equal length
- Minimum trace width: 0.3mm (12 mil)
- Maximum trace length on PCB: <200mm
- Avoid routing near noisy signals (shift register clocks)
- Add ground plane underneath for shielding

---

## Voltage Regulation

### LM7805 Regulator Circuit

**Complete schematic:**
```
[5V Barrel Jack]
    ↓
  [+] Center positive
    ↓
  [1N4001 Diode] ──→ Cathode (band) toward regulator
    ↓
  [470µF Electrolytic] (+) to 5V, (-) to GND
    ↓
  [LM7805 Pin 1 (IN)]
    │
  [LM7805 Pin 2 (GND)] ──→ Ground Plane
    │
  [LM7805 Pin 3 (OUT)]
    ↓
  [10µF Electrolytic] (+) to OUT, (-) to GND
    ↓
  [5V Rail] ──→ To ESP32, shift registers, pull-ups
    ↓
  [100nF Ceramic] VCC to GND (near ESP32)
```

**Component Values:**
- **C1 (Input bulk):** 470µF / 16V electrolytic
- **C2 (Input filter):** 10µF / 16V electrolytic (optional)
- **C3 (Output filter):** 10µF / 16V electrolytic
- **C4 (Output decoupling):** 100nF / 50V ceramic
- **D1 (Reverse protection):** 1N4001 (1A, 50V)

**LM7805 TO-220 Package:**
```
       Front View
       (metal tab)
     ┌─────────────┐
     │             │
     │   LM7805    │
     │             │
     └──┬──┬──┬───┘
        │  │  │
        1  2  3
       IN GND OUT
```

**Heatsink:** Optional for single panel. Required if multiple panels share one regulator.

---

## Decoupling and Filtering

### Shift Register Decoupling

**Every 74HC165 needs a 100nF ceramic capacitor between VCC and GND:**
```
SR1-SR13 pin 16 (VCC) ──→ 100nF cap ──→ SR pin 8 (GND)
```

**Placement rules:**
- Within 5mm of the IC pins
- Short, wide traces to VCC and GND
- Use vias to ground plane for minimum inductance

### ESP32 Power Filtering

**ESP32 VIN (pin 16) power filtering:**
```
5V Rail ──→ 100nF ceramic cap ──→ ESP32 pin 16 ──→ 100nF cap ──→ GND
```

**Additional filtering (optional but recommended):**
```
5V Rail ──→ 10µF electrolytic cap ──→ ESP32 pin 16 ──→ GND
```

**Why?** ESP32 has RF components (WiFi/BT) that can cause voltage spikes. Decoupling prevents brownouts.

### Power Supply Bulk Capacitance

**470µF capacitor at LM7805 input:**
- Smooths input voltage ripple
- Provides current reservoir during load transients
- Place within 20mm of LM7805

---

## PCB Layout Guidelines

### Layer Stackup (2-Layer Board)

**Top Layer (Component Side):**
- Signal traces: Shift register parallel inputs, encoder connections
- Power traces: 5V distribution to shift registers
- Ground fill: Pour copper in unused areas
- Component placement: ESP32, shift registers, connectors

**Bottom Layer (Solder Side):**
- Ground plane: Solid copper pour (maximize area)
- Return paths: Keep under signal traces when possible
- Power distribution: 5V rail if needed

### Trace Width Guidelines

| Signal Type | Current | Trace Width (1oz copper) | Notes |
|-------------|---------|--------------------------|-------|
| Ground | N/A | Pour / Polygon | Maximize copper area |
| 5V Power | 300mA | 0.5mm (20 mil) | Can be wider for safety |
| I2C (SDA, SCL) | 1mA | 0.3mm (12 mil) | Keep equal length |
| Shift register CLK, LATCH | <1mA | 0.25mm (10 mil) | Can be narrow |
| Shift register data | <1mA | 0.25mm (10 mil) | Single-ended |
| Encoder signals | <1mA | 0.2mm (8 mil) | Can be very narrow |

### Critical Routing Rules

**1. Shift Register Clock Distribution:**
```
ESP32 IO14 (CLK) ──→ Star topology or tree to all SR pin 2
```
- Use identical trace lengths if possible (±5mm tolerance)
- Avoid daisy-chaining (creates timing skew)
- Add 33Ω series resistor at ESP32 if ringing occurs

**2. Shift Register Serial Data Chain:**
```
SR1 Q7 → SR2 DS → SR3 DS → ... → SR13 Q7 → ESP32 IO12
```
- Keep traces short (<100mm between chips)
- Can use narrow traces (8 mil / 0.2mm)
- Route away from CLK and LATCH signals to avoid crosstalk

**3. I2C Routing:**
```
ESP32 SDA/SCL ──→ Straight traces ──→ Connector
```
- Keep SDA and SCL parallel, equal length (±2mm)
- Minimum trace width: 0.3mm (12 mil)
- Add ground plane underneath (reduces EMI)
- Avoid 90° angles (use 45° or curved)

**4. Encoder Wiring:**
```
Encoder pins ──→ Shortest path ──→ Shift register pins
```
- Minimize trace length to reduce noise pickup
- Route away from clock signals
- Use ground fill around encoder traces

### Via Guidelines

- **Diameter:** 0.6mm drill, 1.0mm pad (standard)
- **Ground vias:** Place liberally (every 10-20mm)
- **Signal vias:** Minimize usage (vias add inductance)
- **Thermal relief:** Use for ground connections to aid soldering

### Silkscreen Recommendations

**Label everything:**
- IC designators: U1 (ESP32), U2-U14 (SR1-SR13), U15 (LM7805)
- Encoder numbers: E1-E32
- Button numbers: B1-B4
- Pin headers: SDA, SCL, INT, GND, 5V
- Polarity markings: + for electrolytic caps, band for diode
- Power input: "5V DC" next to barrel jack

### Design Rule Check (DRC) Settings for KiCad 9

**Minimum Clearances:**
- Track-to-track: 0.2mm (8 mil)
- Track-to-pad: 0.2mm (8 mil)
- Pad-to-pad: 0.2mm (8 mil)

**Minimum Trace Width:**
- Signal: 0.2mm (8 mil)
- Power: 0.5mm (20 mil)

**Minimum Drill Size:**
- Vias: 0.3mm
- Through-hole pads: 0.8mm (for IC pins)

---

## Testing and Troubleshooting

### Pre-Power Testing

**Before applying power:**

1. **Visual Inspection:**
   - Check for solder bridges (especially on IC pins)
   - Verify component orientation (IC notches, diode band, electrolytic polarity)
   - Inspect for cold solder joints

2. **Continuity Testing:**
   - **Ground plane:** Test continuity between all ground pins
   - **5V rail:** Test continuity from LM7805 output to ESP32 VIN
   - **I2C lines:** Verify no shorts between SDA, SCL, and GND
   - **Shift register chain:** Verify data line connections (Q7 to DS)

3. **Resistance Testing:**
   - **5V to GND:** Should be >1kΩ (pull-up resistors)
   - **I2C pull-ups:** 4.7kΩ from SDA to 5V, SCL to 5V
   - **Short circuits:** No direct shorts between VCC and GND (<10Ω)

### Power-On Testing

**Step 1: Verify Power Supply**
```
1. Connect 5V power supply to barrel jack
2. Measure voltage at LM7805 input (pin 1): Should be ~5V
3. Measure voltage at LM7805 output (pin 3): Should be 4.9-5.1V
4. Measure voltage at ESP32 pin 16: Should be ~5V
5. Measure voltage at shift register VCC pins: Should be ~5V
```

**Step 2: Test ESP32 Boot**
```
1. Observe ESP32 status LED (if installed)
2. Connect USB-to-serial adapter to ESP32 (TXD0/RXD0)
3. Open serial monitor at 115200 baud
4. Press ESP32 reset button
5. Should see boot messages
```

**Step 3: Test Shift Register Chain**
```
1. Upload test firmware to ESP32
2. Firmware should pulse LATCH and CLK
3. Measure SR13 pin 9 (Q7) with oscilloscope: Should see clock pulses
4. If no signal, check:
   - SR13 VCC and GND
   - ESP32 IO14 (CLK) connection
   - ESP32 IO27 (LATCH) connection
```

**Step 4: Test Encoder Inputs**
```
1. Connect one encoder to appropriate pins
2. Rotate encoder slowly
3. Serial monitor should show CLK/DT state changes
4. Press encoder button, should show SW state change
5. Repeat for each encoder
```

### Common Issues and Solutions

| Problem | Cause | Solution |
|---------|-------|----------|
| No power at LM7805 output | Diode reversed | Flip D1 (band toward regulator) |
| ESP32 won't boot | Insufficient power | Check LM7805 heatsink, add capacitors |
| No shift register data | Chain broken | Check Q7→DS connections with multimeter |
| Encoders not detected | Missing pull-ups | Verify 10kΩ resistors on CLK/DT/SW |
| I2C not working | Missing pull-ups or wrong pins | Check 4.7kΩ on SDA/SCL, verify IO21/IO22 |
| Erratic encoder readings | Noise/bounce | Add 100nF caps to encoder pins |
| Some encoders don't work | Solder bridge or broken trace | Inspect SR pins under microscope |

### Debugging Tools Needed

- **Multimeter:** Voltage, continuity, resistance testing
- **Oscilloscope (optional):** For viewing clock signals and timing
- **USB-to-serial adapter:** For ESP32 serial debugging
- **Logic analyzer (optional):** For debugging I2C and shift register data

---

## Assembly Order

**Recommended sequence to minimize rework:**

1. **Power section:**
   - Solder barrel jack
   - Solder 1N4001 diode (check polarity!)
   - Solder LM7805 (check orientation!)
   - Solder 470µF, 10µF capacitors (check polarity!)
   - **TEST:** Verify 5V output before proceeding

2. **Shift registers:**
   - Solder all 13 × 74HC165 ICs (check orientation!)
   - Solder 100nF decoupling caps (one per IC)
   - Solder shift register interconnect wires (Q7 to DS chain)
   - Solder common clock and latch connections

3. **ESP32:**
   - Solder 30-pin DIP socket (recommended over direct soldering)
   - Solder 100nF decoupling cap near VIN
   - Solder I2C pull-up resistors (4.7kΩ)
   - **TEST:** Power on, verify ESP32 boots

4. **Encoders and buttons:**
   - Solder encoder pin headers (or direct wire)
   - Solder 10kΩ pull-up resistors (100 total)
   - Solder button pin headers
   - Connect encoders for testing

5. **Connectors:**
   - Solder I2C 5-pin header
   - Solder any additional headers

6. **Final testing:**
   - Upload firmware
   - Test all encoders and buttons
   - Verify I2C communication

---

## KiCad 9 Design Workflow

### Project Setup

**1. Create New Project:**
```
File → New Project
Name: MIDI_Kraken_32_Encoder_Panel
```

**2. Configure Board Settings:**
```
File → Board Setup
Constraints:
  - Min clearance: 0.2mm
  - Min track width: 0.2mm
  - Min via size: 0.6mm drill, 1.0mm diameter

Net Classes:
  - Default: 0.25mm width
  - Power: 0.5mm width
  - I2C: 0.3mm width
```

### Schematic Entry

**Symbol Library Requirements:**
- 74HC165 (Logic_74xx library)
- LM7805 (Regulator_Linear library)
- ESP32-DevKitC (create custom symbol or use generic 30-pin header)
- Rotary encoder (Device library or custom)
- Tactile switch (Switch library)
- Resistor, capacitor (Device library)
- Connectors (Connector library)

**Hierarchical Design (Recommended):**
```
Top Sheet:
  ├─ Power Supply (sub-sheet)
  ├─ ESP32 Interface (sub-sheet)
  ├─ Shift Register Chain (sub-sheet)
  ├─ Encoder Bank 1 (E1-E8, sub-sheet)
  ├─ Encoder Bank 2 (E9-E16, sub-sheet)
  ├─ Encoder Bank 3 (E17-E24, sub-sheet)
  ├─ Encoder Bank 4 (E25-E32, sub-sheet)
  └─ I2C Connector (sub-sheet)
```

**Net Naming Convention:**
- Power: `+5V`, `GND`
- I2C: `SDA`, `SCL`, `I2C_INT`
- Shift registers: `SR_CLK`, `SR_LATCH`, `SR_DATA`
- Encoders: `E1_CLK`, `E1_DT`, `E1_SW`, etc.

### PCB Layout Process

**1. Import from Schematic:**
```
Tools → Update PCB from Schematic (F8)
```

**2. Define Board Outline:**
```
Edge.Cuts layer:
  - Draw 300mm × 120mm rectangle
  - Add 4 × M3 mounting holes at corners (3.2mm drill)
  - Add fiducials (1mm copper circles) for assembly
```

**3. Place Components (Recommended Order):**
- Power section (left edge)
- ESP32 socket (center-left)
- I2C connector (left edge, near ESP32)
- Shift registers (top edge, linear arrangement)
- Encoder grid (center/right)
- Standalone buttons (bottom)

**4. Route Power and Ground:**
```
Bottom layer: Ground plane (polygon pour)
Top layer: 5V power distribution
  - Use 0.5mm (20 mil) traces
  - Star topology from LM7805 to loads
```

**5. Route Signals:**
```
Shift register chain:
  - Route Q7→DS connections on top layer
  - Keep traces <100mm, parallel routing

Encoder connections:
  - Route to nearest shift register pins
  - Minimize trace length
  - Use ground fill around signals

I2C:
  - Route SDA and SCL parallel, equal length
  - 0.3mm width, keep straight
```

**6. Add Copper Pours:**
```
Top layer: GND fill in unused areas (clearance 0.3mm)
Bottom layer: GND polygon (solid, clearance 0.3mm)
```

**7. Add Silkscreen:**
- Component designators
- Pin labels on connectors
- Polarity markings
- Board name and version

**8. Run Design Rule Check (DRC):**
```
Tools → Design Rules Checker
  - Fix all errors (shorts, clearances)
  - Review warnings
```

**9. Generate Gerber Files:**
```
File → Plot
Output directory: gerbers/
Layers to plot:
  - F.Cu (top copper)
  - B.Cu (bottom copper)
  - F.SilkS (top silkscreen)
  - B.SilkS (bottom silkscreen)
  - F.Mask (top soldermask)
  - B.Mask (bottom soldermask)
  - Edge.Cuts (board outline)

Generate Drill Files:
  - PTH (plated through-hole)
  - NPTH (non-plated, for mounting holes)
```

**10. Generate Assembly Files:**
```
File → Fabrication Outputs → Component Placement
  - CSV format for pick-and-place
  - Include reference, value, footprint, position
```

### PCB Manufacturing

**Recommended Manufacturers:**
- **JLCPCB:** ~$10-15 for 5 boards (2-layer, 300×120mm)
- **PCBWay:** Similar pricing, good quality
- **OSH Park:** Higher quality, more expensive (~$50)

**Specifications to Order:**
- Layers: 2
- Dimensions: 300mm × 120mm
- PCB thickness: 1.6mm (standard)
- Copper weight: 1oz (35µm)
- Soldermask color: Green (or preference)
- Silkscreen color: White
- Surface finish: HASL (lead-free) or ENIG
- Min trace/space: 0.2mm / 0.2mm

---

## Bill of Materials Summary

**Quick reference for ordering:**

| Category | Total Quantity | Estimated Cost |
|----------|----------------|----------------|
| ESP32 DevKit | 1 | $5 |
| 74HC165N | 13 | $6 (0.50 each) |
| LM7805 | 1 | $0.50 |
| KY-050 encoders | 32 | $16 (bulk discount) |
| Tactile switches | 4 | $1 |
| Resistors (10kΩ, 4.7kΩ, 1kΩ) | 52 | $2 |
| Ceramic caps (100nF) | 50 | $3 |
| Electrolytic caps | 4 | $1 |
| Diode (1N4001) | 1 | $0.10 |
| Connectors | 5 | $2 |
| Barrel jack | 1 | $0.50 |
| PCB fabrication | 1 (set of 5) | $10-15 |
| **TOTAL PER PANEL** | | **~$47-52** |

**For 8 synth panels:** ~$376-416 (PCB and components)

---

## Conclusion

This guide provides complete electrical specifications for building a 32-encoder panel. Key points:

1. **Power:** 5V input, LM7805 regulation, 470µF + 10µF filtering
2. **ESP32:** 30-pin DevKit, pins IO12/14/27 for shift registers, IO19/21/22 for I2C
3. **Shift registers:** 13 × 74HC165, daisy-chained Q7→DS, 100nF decoupling each
4. **Encoders:** 32 × KY-050, CLK/DT/SW to shift register inputs, 10kΩ pull-ups
5. **I2C:** 4.7kΩ pull-ups on SDA/SCL, INT line to Teensy
6. **Layout:** 2-layer PCB, ground plane on bottom, signal routing on top

Follow this guide step-by-step for a reliable, professional-quality MIDI controller panel.

**Next Steps:**
1. Create KiCad schematic using this pinout
2. Layout PCB following routing guidelines
3. Order PCB fabrication + components
4. Assemble and test one panel
5. Replicate for remaining 7 panels

Good luck with your build!
