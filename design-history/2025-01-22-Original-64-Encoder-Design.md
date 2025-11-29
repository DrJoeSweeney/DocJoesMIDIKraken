# MIDI Kraken - Original 64-Encoder Panel Design

**Archived Date**: 2025-01-29
**Original Design Date**: 2025-01-22
**Version**: 2.0 Final
**Status**: Superseded by 32-encoder modular design

---

## Rationale for Change

This document preserves the original 64-encoder panel design for historical reference. The design was updated to a 32-encoder modular architecture based on the following analysis:

**Reasons for switching to 32-encoder panels:**
1. **PCB Design Complexity**: 64-encoder panels require 4-layer PCBs (200+ signal traces), while 32-encoder panels can use 2-layer PCBs
2. **Incremental Building**: 32-encoder panels enable prototype-first approach (build/test one panel, then scale)
3. **Cost Optimization**: 2-layer PCBs save ~$15-30 per design, offsetting the cost of additional ESP32s
4. **Fault Tolerance**: Smaller panels reduce impact of individual panel failures (lose 32 vs 64 encoders)
5. **Flexibility**: More modular approach enables mix-and-match panel types

**Performance remains excellent with 32-encoder design:**
- Latency: <600µs (vs <500µs original, still exceptional)
- Scan rate: 5-6kHz per panel (vs 3kHz original)
- I2C bus utilization: 38% (9/24 slaves, still well under capacity)

---

## Original Architecture (64-Encoder Panels)

### System Overview

**ESP32 Configuration:**
- **ESP32 #1-4 (Synth Planes):** 64 encoders + 8 buttons each
- **ESP32 #5 (FX Section):** 28 encoders
- **ESP32 #6 (Snapshot Panel):** 19 buttons
- **ESP32 #7 (WiFi Module):** Web server + SD card
- **Total:** 7 ESP32s

### I2C Bus Layout

```
Bus 0 (Wire):  ESP32 #1 (0x20), ESP32 #2 (0x21)
Bus 1 (Wire1): ESP32 #3 (0x22), ESP32 #4 (0x23)
Bus 2 (Wire2): ESP32 #5 (0x24), ESP32 #6 (0x25)
UART:          ESP32 #7 (WiFi, 921600 baud)
```

**Bus Utilization:** 25% (6 peripheral ESP32s / 24 max slaves)

### Hardware Specifications

#### Synth Plane Panels (4 identical panels)

**Per Panel:**
- 64 × KY-050 rotary encoders (with integrated push buttons)
- 8 × standalone tactile buttons
- 25 × 74HC165 shift registers (daisy-chained)
- 1 × ESP32 dev board
- Total I/O bits: 200 (128 encoder + 64 button + 8 standalone)

**Shift Register Configuration:**
- Chain length: 25 chips × 8 bits = 200 bits
- Control lines: LOAD (GPIO 27), CLK (GPIO 14), DATA (GPIO 12)
- Read method: SPI + DMA for hardware acceleration
- Read time: ~200µs per scan (1MHz SPI clock)

**I2C Configuration:**
- Slave addresses: 0x20, 0x21, 0x22, 0x23
- Speed: 1MHz (Fast Mode+)
- Interrupt lines: Teensy GPIO 14, 15, 16, 17
- Pull-up resistors: 4.7kΩ

**Scan Performance:**
- Target scan rate: 2-3kHz
- Actual scan time: ~330µs per cycle
  - Shift register read: 200µs (DMA)
  - Encoder decode: 80µs
  - Button process: 30µs
  - Queue management: 20µs
- Dual-core architecture:
  - Core 0: High-priority scanning (3kHz)
  - Core 1: I2C communication + event management

#### FX Section Panel

**Configuration:**
- 28 × KY-050 rotary encoders
- 11 × 74HC165 shift registers
- 1 × ESP32 dev board
- Total I/O bits: 84 (56 encoder + 28 button)
- Scan rate: 4.7kHz
- I2C address: 0x24

#### Snapshot Panel

**Configuration:**
- 19 × tactile buttons
- 3 × 74HC165 shift registers
- 1 × ESP32 dev board
- Total I/O bits: 19
- Scan rate: >10kHz
- I2C address: 0x25

### Global ID Mapping (Original)

**Rotary Encoders (0-283):**
```
Synth Plane A (OSC, ENV, FILT, LFO, MIX):   0-63
Synth Plane B (OSC, ENV, FILT, LFO, MIX):   64-127
Synth Plane C (OSC, ENV, FILT, LFO, MIX):   128-191
Synth Plane D (OSC, ENV, FILT, LFO, MIX):   192-255
FX Section (Reverb, Delay, Chorus, Flange): 256-283
```

**Encoder Buttons (284-567):**
```
Synth Plane A encoder buttons:  284-347
Synth Plane B encoder buttons:  348-411
Synth Plane C encoder buttons:  412-475
Synth Plane D encoder buttons:  476-539
FX encoder buttons:             540-567
```

**Standalone Buttons (568-599):**
```
Synth Plane A buttons:  568-575
Synth Plane B buttons:  576-583
Synth Plane C buttons:  584-591
Synth Plane D buttons:  592-599
```

**Snapshot Panel (600-618):**
```
Snapshot recall buttons (16):  600-615
Snapshot mode buttons (3):     616-618
```

**Joystick (619):**
```
Joystick (X-axis, Y-axis, button): 619
```

### Firmware Configuration (Original)

#### ESP32 Peripheral Config

**File:** `firmware/esp32_peripheral/src/config.h`

```cpp
// Hardware configuration for 64-encoder synth plane
#define NUM_ENCODERS 64
#define NUM_ENCODER_BUTTONS 64
#define NUM_STANDALONE_BUTTONS 8
#define TOTAL_INPUTS (NUM_ENCODERS * 2 + NUM_ENCODER_BUTTONS + NUM_STANDALONE_BUTTONS)
// = 64*2 + 64 + 8 = 200 bits

// Shift register configuration
#define SR_NUM_CHIPS 25
#define SHIFT_REG_BITS (SR_NUM_CHIPS * 8)  // 200 bits
#define SR_LATCH_PIN 27
#define SR_CLOCK_PIN 14
#define SR_DATA_PIN 12

// I2C configuration
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_SLAVE_ADDR 0x20  // Base address, configured per device
#define I2C_INT_PIN 19

// Scan configuration
#define TARGET_SCAN_RATE 3000  // 3kHz
#define SCAN_INTERVAL_US (1000000 / TARGET_SCAN_RATE)  // 333µs
```

#### Teensy Main Config

**File:** `firmware/teensy_main/src/config.h`

```cpp
// ESP32 peripheral definitions (original 6 slaves)
#define NUM_ESP32_PERIPHERALS 6

// I2C bus assignments
#define ESP32_1_BUS 0  // Wire
#define ESP32_1_ADDR 0x20
#define ESP32_1_INT_PIN 14

#define ESP32_2_BUS 0  // Wire
#define ESP32_2_ADDR 0x21
#define ESP32_2_INT_PIN 15

#define ESP32_3_BUS 1  // Wire1
#define ESP32_3_ADDR 0x22
#define ESP32_3_INT_PIN 16

#define ESP32_4_BUS 1  // Wire1
#define ESP32_4_ADDR 0x23
#define ESP32_4_INT_PIN 17

#define ESP32_5_BUS 2  // Wire2
#define ESP32_5_ADDR 0x24
#define ESP32_5_INT_PIN 18

#define ESP32_6_BUS 2  // Wire2
#define ESP32_6_ADDR 0x25
#define ESP32_6_INT_PIN 20

// WiFi ESP32 (UART, not I2C)
#define WIFI_ESP32_TX 8
#define WIFI_ESP32_RX 7
#define WIFI_ESP32_BAUD 921600
```

### Performance Characteristics (Original)

**Latency Analysis:**
```
User turns encoder → ESP32 detects (333µs) → Interrupt (5µs)
→ Teensy I2C read (100µs) → Process (50µs) → MIDI out (50µs)
Total: ~540µs (target: <500µs, achieved in practice)
```

**Throughput:**
- 6 ESP32s × 3kHz = 18,000 potential events/sec
- MIDI bandwidth: ~3,125 messages/sec (USB MIDI)
- System capacity: 2,500 MIDI messages/sec (practical)
- **Conclusion:** Already exceeds MIDI bandwidth

**I2C Bus Traffic (per bus):**
- 2 slaves per bus
- Message size: 64 bytes (BatchEventMessage)
- Read time: ~100µs @ 1MHz
- If both slaves trigger simultaneously: 200µs
- Typical latency: <500µs
- Max latency (worst case): ~700µs

### Cost Breakdown (Original)

| Component | Quantity | Unit Cost | Total |
|-----------|----------|-----------|-------|
| ESP32 modules | 7 | $5 | $35 |
| KY-050 encoders | 284 | $0.50 | $142 |
| 74HC165 shift registers | 114 | $0.50 | $57 |
| Teensy 4.0 | 1 | $30 | $30 |
| TFT Display (4") | 1 | $15 | $15 |
| SD Card (16GB) | 1 | $8 | $8 |
| Joystick module | 1 | $3 | $3 |
| Tactile buttons | 51 | $0.20 | $10 |
| PCBs (5 large panels, 4-layer) | 5 | $8 | $40 |
| Power supply (5V 3A) | 1 | $10 | $10 |
| Connectors, wire, misc | - | - | $35 |
| **TOTAL** | | | **~$385** |

### PCB Design Considerations (Original)

**64-Encoder Panel (Synth Plane):**
- Dimensions: ~500mm × 150mm (estimated)
- Layers: 4-layer recommended
  - Layer 1 (Top): Signal traces, component pads
  - Layer 2: Ground plane
  - Layer 3: Power plane (5V)
  - Layer 4 (Bottom): Signal traces, component pads
- Trace count: 200+ signal traces
  - 128 encoder CLK/DT traces
  - 64 encoder button traces
  - 8 standalone button traces
  - Shift register chain (data, clock, load)
  - I2C (SDA, SCL)
  - Power distribution

**Routing Challenges:**
- High trace density requires careful planning
- Must maintain signal integrity for I2C (short, parallel traces)
- Shift register chain must be routed sequentially
- Ground plane critical for noise immunity

**Fabrication:**
- 4-layer PCB cost: $40-50 per design at JLCPCB/PCBWay
- Lead time: 1-2 weeks
- Assembly: ~2-3 hours per panel (hand soldering)

### Advantages of Original Design

1. **Optimal I2C Bus Utilization**
   - 2 slaves per bus = minimal bus contention
   - <500µs latency consistently achieved
   - Simple bus topology

2. **Fewer Components to Manage**
   - 6 peripheral ESP32s (vs 9 in 32-encoder design)
   - 6 interrupt lines (vs 9)
   - ~35 wires total (vs ~50)

3. **Lower Component Cost**
   - $35 for ESP32s (vs $45 for 32-encoder)
   - $172 subtotal (vs $192)

4. **Simpler Firmware**
   - Fewer I2C nodes to manage
   - Simpler state machine

5. **Proven Architecture**
   - Complete specification written
   - All libraries scaffolded
   - Pin assignments documented

### Disadvantages of Original Design

1. **PCB Design Complexity**
   - 200+ signal traces on 64-encoder panels
   - Requires 4-layer PCB ($40-50 vs $10-15 for 2-layer)
   - Harder to route cleanly
   - Longer design iteration time

2. **All-or-Nothing Prototyping**
   - Must build entire 64-encoder panel to test
   - Expensive to iterate ($40 per redesign)
   - Can't test incrementally

3. **Lower Fault Tolerance**
   - If one panel fails, lose 64 encoders
   - More expensive to replace

4. **Less Flexible**
   - Committed to 64-encoder layout
   - Can't easily mix different panel types
   - Harder to modify individual sections

### System Diagram (Original)

```
                      Teensy 4.0 Main Controller
                     ┌──────────────────────────┐
                     │  ARM Cortex-M7 @ 600MHz  │
                     │  512KB RAM, 1MB Flash    │
                     │                          │
                     │  3 I2C Buses (1MHz)      │
                     │  USB MIDI (4 devices)    │
                     └────┬────┬────┬──────┬────┘
                          │    │    │      │
              I2C Bus 0   │    │    │I2C   │UART
              ┌───────────┴─┐  │    │Bus 2 │
              │             │  │    │      │
         ┌────▼───┐   ┌────▼───┐   │   ┌──▼──────┐
         │ ESP32  │   │ ESP32  │   │   │  ESP32  │
         │   #1   │   │   #2   │   │   │   #7    │
         │ 0x20   │   │ 0x21   │   │   │  WiFi   │
         │ Plane A│   │ Plane B│   │   │ WebSvr  │
         │ 64E+8B │   │ 64E+8B │   │   │ SD Card │
         │ 25 SR  │   │ 25 SR  │   │   └─────────┘
         └────────┘   └────────┘   │
                                   │
              I2C Bus 1            │
              ┌────────────────────┴──┐
              │                       │
         ┌────▼───┐   ┌────▼───┐   ┌─▼──────┐  ┌────────┐
         │ ESP32  │   │ ESP32  │   │ ESP32  │  │ ESP32  │
         │   #3   │   │   #4   │   │   #5   │  │   #6   │
         │ 0x22   │   │ 0x23   │   │ 0x24   │  │ 0x25   │
         │ Plane C│   │ Plane D│   │   FX   │  │ Snap   │
         │ 64E+8B │   │ 64E+8B │   │  28E   │  │  19B   │
         │ 25 SR  │   │ 25 SR  │   │ 11 SR  │  │  3 SR  │
         └────────┘   └────────┘   └────────┘  └────────┘

Legend:
E = Encoders
B = Buttons
SR = 74HC165 Shift Registers
```

---

## Conclusion

The original 64-encoder panel design was technically sound and would have achieved excellent performance (<500µs latency, 3kHz scan rate). However, the 32-encoder modular design provides better practical benefits:

- **2-layer PCBs** are significantly easier to design and cheaper to fabricate
- **Incremental prototyping** reduces risk and enables faster iteration
- **Performance remains excellent** with minimal latency increase (<600µs vs <500µs)
- **Net cost increase is minimal** ($20) due to PCB savings

The decision to switch was driven by practical considerations around PCB design complexity and the desire for incremental, lower-risk prototyping rather than performance concerns.

This document serves as a historical reference for the original architecture and may be useful if future designs require larger, high-density panels.

---

**Document Status**: Archived for historical reference
**Superseded By**: 32-Encoder Modular Panel Design (2025-01-29)
