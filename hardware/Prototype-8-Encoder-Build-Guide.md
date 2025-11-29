# MIDI Kraken Prototype: 8-Encoder Test Build

**Version**: 1.0
**Date**: 2025-01-29
**Purpose**: Minimal working prototype to validate design before building full 32-encoder panels

---

## Overview

This guide walks you through building a compact 8-encoder MIDI controller prototype using:
- **ESP32S-38P with expansion harness board** (your existing hardware)
- **1× 74HC165 3-chip breakout board** (24 inputs = 8 encoders × 3 pins each)
- **8× KY-050 rotary encoders** (or any quadrature encoders with push buttons)
- **3.3V operation** (single voltage, simplified power system)

**Build time:** 2-4 hours
**Cost:** ~$25-30 (if purchasing all new components)

---

## Why Start with 8 Encoders?

✅ **Validate shift register chain** - Test daisy-chaining before scaling up
✅ **Test firmware** - Verify encoder reading, debouncing, and I2C communication
✅ **Verify power system** - Confirm 3.3V operation with HC chips
✅ **Quick iteration** - Fix issues on small scale before committing to 32-encoder panels
✅ **Breadboard-friendly** - Entire system fits on one breadboard
✅ **Cost-effective** - Test with minimal component investment

---

## Bill of Materials

### Hardware Components

| Qty | Part | Description | Notes |
|-----|------|-------------|-------|
| 1 | ESP32S-38P Harness Board | Your existing board | With expansion headers |
| 1 | 74HC165 3-chip breakout | Shift register board | 24 inputs (8 encoders × 3 pins) |
| 8 | KY-050 rotary encoder | Quadrature with button | Or equivalent |
| 1 | LM1117-3.3 | 3.3V voltage regulator | SOT-223 or TO-220, 800mA |
| 2 | 10µF electrolytic cap | Regulator input/output | 16V or higher |
| 2 | 100nF ceramic cap | Decoupling | 50V |
| 24 | 10kΩ resistor | Encoder pull-ups | 1/4W, to 3.3V |
| 1 | Breadboard | 830 tie-points | Full-size recommended |
| 1 | 5V power supply | USB adapter or bench supply | 1A minimum |
| 1 | Jumper wire set | Male-male, male-female | Various lengths |
| 1 | Multimeter | For voltage checks | Essential! |

**Optional but recommended:**
- Breadboard power supply module (5V → 3.3V conversion)
- Extra breadboards for encoder mounting
- USB cable for ESP32 programming

### Tools Required

- Wire strippers
- Small screwdrivers
- Needle-nose pliers
- Multimeter

---

## System Architecture

```
                    ┌─────────────────────┐
[5V Power Supply] ──→│   LM1117-3.3       │
                    │   (Voltage Reg)    │
                    └─────────────────────┘
                            │
                        [3.3V Rail]
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
┌───────▼─────────┐ ┌──────▼──────┐ ┌──────────▼────────┐
│  ESP32S-38P     │ │  3-Chip     │ │  8× Encoders      │
│  Harness Board  │ │  74HC165    │ │  (24 pins total)  │
│                 │ │  Breakout   │ │                   │
│  P27 → LATCH ───┼─→ /PL         │ │  CLK, DT, SW      │
│  P14 → CLK ─────┼─→ CLK         │ │  per encoder      │
│  P12 ← DATA ────┼─← Q7          │ │                   │
│                 │ │             │ │                   │
│  5V IN ─────────┤ │  VCC ───────┤ │  Pull-ups to 3.3V │
│  GND ───────────┴─┴─ GND ───────┴─┴─ GND rail         │
└─────────────────┘ └─────────────┘ └───────────────────┘
```

**Key Design Points:**
- **Single 3.3V rail** - Simplifies power distribution
- **One breakout board** - Minimal complexity for testing
- **8 encoders** - Enough to validate firmware without overwhelming breadboard
- **Harness board power** - Uses built-in connectors for clean wiring

---

## Build Instructions

### Step 1: Power System Setup

**1.1 Build 3.3V Power Rail**

```
Breadboard Layout:

[+5V Rail] ────┬──→ LM1117-3.3 IN (pin 3)
               │
           [10µF cap]
               │
           [LM1117-3.3]
            Pin 1: GND
            Pin 2: OUT (3.3V)
            Pin 3: IN (5V)
               │
           [10µF cap]
               │
           [3.3V Rail] ──→ Distribute to components

[GND Rail] ─────→ Common ground
```

**Wiring Steps:**
1. Connect 5V power supply + to top breadboard rail → mark as **+5V**
2. Connect 5V power supply GND to bottom breadboard rail → mark as **GND**
3. Insert LM1117-3.3 regulator (check datasheet for pin orientation)
4. Solder 10µF cap between +5V rail and GND (input side, mind polarity!)
5. Wire LM1117 pin 3 (IN) to +5V rail
6. Wire LM1117 pin 1 (GND) to GND rail
7. Solder 10µF cap between LM1117 pin 2 (OUT) and GND (output side)
8. Create new **+3.3V rail** from LM1117 pin 2 output

**1.2 Test Power System**

```bash
Power on → Measure with multimeter:
✅ +5V rail: 4.8-5.2V
✅ +3.3V rail: 3.2-3.4V
✅ If incorrect, CHECK REGULATOR ORIENTATION!
```

**⚠️ DO NOT PROCEED if voltages are incorrect!**

---

### Step 2: Connect ESP32S-38P Harness Board

**2.1 Power Connections**

```
+5V Rail ──→ Harness "5V" header (top power section)
GND Rail ──→ Harness "GND" headers (use at least 2 for stability)
```

**2.2 Test ESP32 Power**

```
✅ ESP32 power LED lights up
✅ Harness board DC jack LED (if present)
✅ Connect USB cable
✅ Upload "Blink" sketch to verify ESP32 works
```

**Arduino IDE Test:**
```cpp
void setup() {
  pinMode(2, OUTPUT);  // Built-in LED on some boards
}

void loop() {
  digitalWrite(2, HIGH);
  delay(500);
  digitalWrite(2, LOW);
  delay(500);
}
```

---

### Step 3: Connect 74HC165 Breakout Board

**3.1 Power the Breakout Board (CRITICAL: Use 3.3V!)**

```
+3.3V Rail ──→ Breakout Board VCC pin
GND Rail   ──→ Breakout Board GND pin
```

**3.2 Connect Control Signals**

```
Harness "P27" header ──→ Breakout Board /PL (LATCH)
Harness "P14" header ──→ Breakout Board CLK (CLOCK)
Harness "P12" header ──→ Breakout Board Q7 (DATA OUT, last chip)
```

**3.3 Pull-up LATCH Signal**

```
Breakout /PL pin ──┬──→ Harness "P27"
                   └──→ 10kΩ resistor ──→ +3.3V Rail
```

**3.4 Ground First Chip DS Input**

```
Breakout Board DS pin (first chip) ──→ GND Rail (or via 10kΩ)
```

**3.5 Add Decoupling Caps**

```
Near each chip on breakout board:
  100nF ceramic cap between VCC and GND
```

**3.6 Test Shift Register Power**

```
✅ Measure breakout VCC: Should be 3.2-3.4V
✅ No smoke, no excessive heat
✅ Chips should be cool to touch
```

---

### Step 4: Connect 8 Encoders

**4.1 Encoder Pinout (KY-050 Standard)**

```
Encoder Pins:
  CLK  → Quadrature channel A
  DT   → Quadrature channel B
  SW   → Push button (active low)
  +    → Not used (internal pull-ups not needed)
  GND  → Ground
```

**4.2 Wiring Pattern (3 pins per encoder)**

**Encoder #1:**
```
Encoder CLK ──┬──→ Breakout D0
              └──→ 10kΩ ──→ +3.3V

Encoder DT  ──┬──→ Breakout D1
              └──→ 10kΩ ──→ +3.3V

Encoder SW  ──┬──→ Breakout D2
              └──→ 10kΩ ──→ +3.3V

Encoder GND ──→ GND Rail
```

**Encoder #2:**
```
CLK → D3 + 10kΩ to 3.3V
DT  → D4 + 10kΩ to 3.3V
SW  → D5 + 10kΩ to 3.3V
GND → GND Rail
```

**Continue pattern for encoders #3-8:**
- Encoder #3: D6, D7, D8
- Encoder #4: D9, D10, D11 (next chip on breakout)
- Encoder #5: D12, D13, D14
- Encoder #6: D15, D16, D17 (next chip)
- Encoder #7: D18, D19, D20
- Encoder #8: D21, D22, D23

**⚠️ CRITICAL:** All pull-ups MUST go to **+3.3V rail**, NOT 5V!

**4.3 Encoder Mounting Tips**

- Use small breadboards to mount encoders
- Keep wires short (<15cm) to reduce noise
- Twist CLK/DT pairs together to minimize crosstalk
- Label each encoder (1-8) with tape or marker

---

### Step 5: Upload Test Firmware

**5.1 Create Arduino Sketch**

Create new sketch: `MIDI_Kraken_8Encoder_Test.ino`

```cpp
// MIDI Kraken 8-Encoder Prototype Test
// ESP32S-38P Harness Board + 74HC165 3-chip breakout @ 3.3V

#define LATCH_PIN 27  // Harness P27
#define CLK_PIN   14  // Harness P14
#define DATA_PIN  12  // Harness P12

#define NUM_CHIPS 3   // 3-chip breakout board
#define NUM_ENCODERS 8

// Encoder state tracking
struct Encoder {
  uint8_t clkBit;
  uint8_t dtBit;
  uint8_t swBit;
  uint8_t lastState;
  int32_t position;
  bool buttonPressed;
};

Encoder encoders[NUM_ENCODERS] = {
  {0, 1, 2, 0, 0, false},   // Encoder 1: D0, D1, D2
  {3, 4, 5, 0, 0, false},   // Encoder 2: D3, D4, D5
  {6, 7, 8, 0, 0, false},   // Encoder 3: D6, D7, D8
  {9, 10, 11, 0, 0, false}, // Encoder 4: D9, D10, D11
  {12, 13, 14, 0, 0, false},// Encoder 5: D12, D13, D14
  {15, 16, 17, 0, 0, false},// Encoder 6: D15, D16, D17
  {18, 19, 20, 0, 0, false},// Encoder 7: D18, D19, D20
  {21, 22, 23, 0, 0, false} // Encoder 8: D21, D22, D23
};

uint8_t shiftData[NUM_CHIPS];

void setup() {
  Serial.begin(115200);

  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);

  digitalWrite(LATCH_PIN, HIGH);
  digitalWrite(CLK_PIN, LOW);

  delay(100);

  Serial.println("====================================");
  Serial.println("MIDI Kraken 8-Encoder Prototype");
  Serial.println("ESP32S-38P + 74HC165 @ 3.3V");
  Serial.println("====================================");
  Serial.println();
  Serial.println("Rotate encoders and press buttons...");
  Serial.println();
}

void loop() {
  // Read shift registers
  readShiftRegisters();

  // Process each encoder
  for (int i = 0; i < NUM_ENCODERS; i++) {
    processEncoder(i);
  }

  delay(1);  // 1ms loop time = 1kHz scan rate
}

void readShiftRegisters() {
  // Latch parallel data
  digitalWrite(LATCH_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(5);

  // Shift in data from all chips
  for (int chip = 0; chip < NUM_CHIPS; chip++) {
    shiftData[chip] = 0;
    for (int bit = 0; bit < 8; bit++) {
      shiftData[chip] |= (digitalRead(DATA_PIN) << bit);

      digitalWrite(CLK_PIN, HIGH);
      delayMicroseconds(5);
      digitalWrite(CLK_PIN, LOW);
      delayMicroseconds(5);
    }
  }
}

bool getBit(uint8_t bitNumber) {
  uint8_t chip = bitNumber / 8;
  uint8_t bit = bitNumber % 8;
  return (shiftData[chip] >> bit) & 1;
}

void processEncoder(uint8_t encNum) {
  Encoder* enc = &encoders[encNum];

  // Read current state
  uint8_t clk = getBit(enc->clkBit);
  uint8_t dt = getBit(enc->dtBit);
  uint8_t sw = getBit(enc->swBit);

  // Quadrature decoding
  uint8_t currentState = (clk << 1) | dt;
  uint8_t lastState = enc->lastState;

  if (currentState != lastState) {
    // Gray code sequence: 00 → 01 → 11 → 10 → 00 (CW)
    //                     00 → 10 → 11 → 01 → 00 (CCW)

    if ((lastState == 0 && currentState == 1) ||
        (lastState == 1 && currentState == 3) ||
        (lastState == 3 && currentState == 2) ||
        (lastState == 2 && currentState == 0)) {
      enc->position++;
      Serial.printf("Encoder %d: CW → Position %d\n", encNum + 1, enc->position);
    }
    else if ((lastState == 0 && currentState == 2) ||
             (lastState == 2 && currentState == 3) ||
             (lastState == 3 && currentState == 1) ||
             (lastState == 1 && currentState == 0)) {
      enc->position--;
      Serial.printf("Encoder %d: CCW → Position %d\n", encNum + 1, enc->position);
    }

    enc->lastState = currentState;
  }

  // Button handling (active low, with pull-up)
  bool buttonPressed = !sw;  // Invert since pull-up makes idle = HIGH

  if (buttonPressed && !enc->buttonPressed) {
    Serial.printf("Encoder %d: BUTTON PRESSED (pos=%d)\n", encNum + 1, enc->position);
  }
  else if (!buttonPressed && enc->buttonPressed) {
    Serial.printf("Encoder %d: BUTTON RELEASED\n", encNum + 1);
  }

  enc->buttonPressed = buttonPressed;
}
```

**5.2 Upload and Test**

1. Select board: "ESP32 Dev Module"
2. Select port: (your ESP32 COM port)
3. Upload sketch
4. Open Serial Monitor (115200 baud)

**Expected Output:**
```
====================================
MIDI Kraken 8-Encoder Prototype
ESP32S-38P + 74HC165 @ 3.3V
====================================

Rotate encoders and press buttons...

Encoder 1: CW → Position 1
Encoder 1: CW → Position 2
Encoder 1: BUTTON PRESSED (pos=2)
Encoder 1: BUTTON RELEASED
Encoder 2: CCW → Position -1
...
```

---

## Validation Checklist

### Power System
- [ ] 3.3V rail measures 3.2-3.4V under load
- [ ] No voltage drop when encoders are connected
- [ ] Regulator stays cool (not hot to touch)
- [ ] No noise or instability on 3.3V rail

### Shift Registers
- [ ] All 24 inputs read correctly (test with jumper wires first)
- [ ] Data chain shifts properly through all 3 chips
- [ ] No intermittent failures during continuous scanning
- [ ] Chips remain cool during operation

### Encoders
- [ ] All 8 encoders respond to rotation (CW and CCW)
- [ ] Quadrature decoding works (no missed steps)
- [ ] Button presses detected on all 8 encoders
- [ ] No false triggers when idle
- [ ] Fast rotation doesn't miss steps (<1 error per 100 detents)

### Performance
- [ ] Scan rate: 500-1000 Hz minimum (check Serial output timing)
- [ ] No serial output lag when multiple encoders turn simultaneously
- [ ] System stable for >10 minutes continuous operation
- [ ] No crashes or resets

---

## Troubleshooting

### Problem: All Inputs Read HIGH (1)

**Cause:** Missing or incorrect pull-ups
**Solution:**
- Verify 10kΩ resistors from encoder pins to +3.3V rail
- Check continuity with multimeter
- Ensure pull-ups go to 3.3V, NOT 5V or floating

### Problem: All Inputs Read LOW (0)

**Cause:** Wrong voltage on shift registers or shorts
**Solution:**
- Verify breakout VCC = 3.3V (not 5V!)
- Check encoder GND connections
- Look for solder bridges on breakout board
- Test with encoders disconnected (should read HIGH)

### Problem: Encoders Count Incorrectly

**Cause:** Noise, debounce issues, or wrong CLK/DT wiring
**Solution:**
- Add 100nF caps close to each chip VCC/GND
- Verify CLK and DT not swapped
- Shorten wire lengths
- Twist CLK/DT wires together
- Try lower scan rate (increase delay in loop)

### Problem: Some Encoders Work, Others Don't

**Cause:** Wiring errors, bad encoders, or incorrect bit mapping
**Solution:**
- Use Serial.print to show raw shift register data
- Verify each encoder connected to correct Dx pins
- Test suspect encoders with multimeter (resistance check)
- Swap encoders to isolate hardware vs wiring issues

### Problem: Buttons Don't Register

**Cause:** Inverted logic or missing pull-ups
**Solution:**
- Verify SW pin pull-up to 3.3V
- Check if button needs logic inversion (active low vs high)
- Test button with Serial.print of raw bit value
- Some KY-050 clones have inverted button logic

### Problem: ESP32 Won't Program

**Cause:** USB cable, driver, or boot mode issue
**Solution:**
- Use data-capable USB cable (not charge-only)
- Install CP2102 or CH340 drivers
- Hold BOOT button on ESP32 while clicking Upload
- Try different USB port or computer

### Problem: Intermittent Failures

**Cause:** Power supply issues or loose connections
**Solution:**
- Add bulk capacitor (470µF) on 5V input
- Add 100µF capacitor on 3.3V rail
- Check all jumper wire connections (reseat firmly)
- Verify breadboard spring contacts (old breadboards fail)
- Measure 3.3V rail under load (shouldn't drop below 3.1V)

---

## Next Steps After Validation

Once your 8-encoder prototype works perfectly:

### Option 1: Scale to 32 Encoders (Full Panel)
- Add 3 more breakout boards (total 13 chips)
- Daisy-chain data: Q7 → DS between boards
- Follow `PCB-Builders-Guide-Alternative-Breakout-Boards.md`
- Design PCB to mount all components

### Option 2: Add I2C Multi-Panel Support
- Implement I2C slave on ESP32
- Connect to Teensy 4.0 master controller
- Follow `I2C-Address-Reference.md` for addressing
- Test with second ESP32 prototype (address 0x09)

### Option 3: Build Multiple 8-Encoder Modules
- Replicate prototype as modular controllers
- Each module = standalone USB MIDI device
- Chain multiple modules via USB hub
- Simpler than multi-panel I2C architecture

### Option 4: PCB Design
- Capture schematic in KiCad
- Use `ESP32S-38P-Harness.kicad_sym` symbol
- Use `ESP32S-38P-Harness.kicad_mod` footprint
- Layout 8-encoder PCB (compact desktop controller)
- Order from JLCPCB/PCBWay (~$10 for 5 boards)

---

## Shopping List (New Purchase)

If ordering everything from scratch:

### Electronics Distributor (DigiKey, Mouser, Amazon)
```
[ ] LM1117-3.3 regulator (SOT-223) - Qty: 1 - ~$1
[ ] 10µF electrolytic caps (16V) - Qty: 2 - ~$0.50
[ ] 100nF ceramic caps (50V) - Qty: 4 - ~$0.50
[ ] 10kΩ resistors (1/4W) - Qty: 30 (24 + spares) - ~$2
[ ] Breadboard (830 points) - Qty: 1 - ~$5
[ ] Jumper wire kit - Qty: 1 kit - ~$8
[ ] 5V power supply (2A USB adapter) - Qty: 1 - ~$5
```

### Encoders (Amazon, AliExpress, eBay)
```
[ ] KY-050 rotary encoders - Qty: 8 - ~$8-10
    Or: Generic EC11 encoders with push button
```

### Breakout Boards (Amazon, AliExpress)
```
[ ] 74HC165 3-chip breakout board - Qty: 1 - ~$3-5
    Search: "74HC165 shift register breakout"
```

**Total estimated cost: $30-35** (assuming you have ESP32S-38P already)

---

## Time Estimates

**First-time builder:**
- Component gathering: 30 min
- Power system build: 30 min
- ESP32 connection: 15 min
- Shift register wiring: 30 min
- Encoder wiring: 60-90 min
- Firmware upload: 15 min
- Testing and debug: 30-60 min
- **Total: 3-4 hours**

**Experienced builder:**
- All steps: 90-120 min

---

## Success Criteria

Your prototype is ready for scaling up when:

✅ All 8 encoders respond perfectly (CW, CCW, button)
✅ No missed steps during fast rotation
✅ Stable for >30 minutes continuous operation
✅ Scan rate >500 Hz
✅ 3.3V rail stable under full load
✅ Serial output shows clean data (no noise/glitches)
✅ You understand the entire signal flow

**Congratulations! You've validated the MIDI Kraken design.** 🎛️

Time to scale up to 32 encoders or design your first PCB!

---

## Support

**Issues or questions?**
- Review `Getting-Started-First-Build.md` for detailed troubleshooting
- Check `ESP32-38Pin-Pinout-Reference.md` for harness pinout
- See `PCB-Builders-Guide-Alternative-Breakout-Boards.md` for scaling up
- Post issues on GitHub: https://github.com/anthropics/DocJoesMIDIKraken/issues

Good luck with your build! 🚀
