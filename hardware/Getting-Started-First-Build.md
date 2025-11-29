# Getting Started: Your First MIDI Kraken Panel Build

**Version**: 1.1
**Date**: 2025-01-29
**Your Hardware**: ESP32-S 38-pin with expansion harness + 74HC165 breakout boards

**Note:** This guide includes instructions for both standard ESP32-DevKit boards and the ESP32-S 38P with expansion harness. Follow "Option B" instructions throughout if you have the harness board.

---

## Build Strategy Overview

**Phase 1: Breadboard Prototype** (1-2 days)
- Test 3.3V power system
- Verify shift register chain
- Test with 4-8 encoders
- Validate firmware

**Phase 2: First PCB Panel** (1-2 weeks)
- Design KiCad schematic
- Layout PCB for your boards
- Order fabrication + components
- Assemble and test

**Phase 3: Production** (Ongoing)
- Build remaining 7 panels
- Integrate with Teensy
- Full system testing
- Enclosure design

---

## Phase 1: Breadboard Prototype (START HERE)

### Goal
Validate your 74HC165 breakout boards work at 3.3V before committing to PCB design.

### What You Need

**Hardware (Minimal Test Setup):**
- [ ] 1 × ESP32-S 38-pin board
- [ ] 1 × 74HC165 breakout board (3 or 4 chip version)
- [ ] 4-8 × KY-050 rotary encoders (or any quadrature encoders)
- [ ] 1 × LM1117-3.3 voltage regulator (SOT-223 or TO-220)
- [ ] 1 × 5V power supply (USB, wall adapter, or bench supply)
- [ ] 2 × 10µF electrolytic capacitors (for regulator)
- [ ] 1 × breadboard (830 tie-points minimum)
- [ ] Jumper wires (male-male, male-female)
- [ ] 8-16 × 10kΩ resistors (encoder pull-ups to 3.3V)
- [ ] 2 × 4.7kΩ resistors (I2C pull-ups - if needed later)
- [ ] Multimeter (for voltage checks)

**Software:**
- [ ] Arduino IDE or PlatformIO
- [ ] ESP32 board support package
- [ ] USB cable for ESP32 programming

### Step 1: Build 3.3V Power Supply

**Breadboard Layout:**
```
Power Rails:
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
            [3.3V Rail] ──→ Distribute to boards

[GND Rail] ─────→ Common ground
```

**Wiring:**
1. Connect 5V power supply + to breadboard top rail (mark as +5V)
2. Connect 5V power supply GND to breadboard bottom rail (mark as GND)
3. Insert LM1117-3.3 regulator into breadboard
4. Connect 10µF cap between 5V rail and GND (input side)
5. Wire LM1117 pin 3 (IN) to +5V rail
6. Wire LM1117 pin 1 (GND) to GND rail
7. Connect 10µF cap between LM1117 pin 2 (OUT) and GND
8. Create new 3.3V rail from LM1117 pin 2 output

**Test:**
```
Power on → Measure with multimeter:
✅ 5V rail: 4.8-5.2V
✅ 3.3V rail: 3.2-3.4V
✅ If incorrect, check regulator orientation and caps
```

### Step 2: Connect ESP32

**Power ESP32 from 5V:**

**Option A: Standard ESP32-DevKit (no harness):**
```
+5V Rail ──→ ESP32 Pin 37 (VIN)
GND Rail ──→ ESP32 Pin 38 (GND)
GND Rail ──→ ESP32 Pin 31 (GND)
```

**Option B: ESP32-S 38P with Harness Board (your board):**
```
+5V Rail ──→ Harness "5V" header (top power section)
GND Rail ──→ Harness "GND" headers (multiple available)
```

**Test:**
```
✅ ESP32 power LED lights up
✅ DC3.5 barrel jack LED lights up (if present)
✅ Connect USB cable
✅ Upload "Blink" example sketch
✅ Verify ESP32 works
```

### Step 3: Connect One 74HC165 Breakout Board

**Power the board from 3.3V (NOT 5V!):**
```
3.3V Rail ──→ Breakout Board VCC
GND Rail  ──→ Breakout Board GND
```

**Connect control signals:**

**Option A: Standard ESP32-DevKit (no harness):**
```
ESP32 Pin 11 (IO27) ──→ Board /PL (LATCH)
ESP32 Pin 12 (IO14) ──→ Board CLK (CLOCK)
ESP32 Pin 13 (IO12) ──→ Board Q7 (DATA OUT)
```

**Option B: ESP32-S 38P with Harness Board (your board):**
```
Harness "P27" header ──→ Board /PL (LATCH)
Harness "P14" header ──→ Board CLK (CLOCK)
Harness "P12" header ──→ Board Q7 (DATA OUT)
```

**Add 10kΩ pull-up on LATCH:**

**Option A: Standard ESP32-DevKit:**
```
Board /PL pin ──┬──→ ESP32 Pin 11 (IO27)
                └──→ 10kΩ resistor ──→ 3.3V Rail
```

**Option B: Harness Board:**
```
Board /PL pin ──┬──→ Harness "P27" header
                └──→ 10kΩ resistor ──→ 3.3V Rail
```

**First chip DS input:**
```
Board DS (first chip) ──→ GND (or 10kΩ to GND)
```

**Test:**
```
✅ Measure board VCC: Should be 3.2-3.4V
✅ No smoke or excessive heat
```

### Step 4: Connect Encoders to Board

**For first encoder (CLK, DT, SW):**
```
Encoder CLK pin ──┬──→ Board D0 input
                  └──→ 10kΩ resistor ──→ 3.3V Rail

Encoder DT pin  ──┬──→ Board D1 input
                  └──→ 10kΩ resistor ──→ 3.3V Rail

Encoder SW pin  ──┬──→ Board D2 input
                  └──→ 10kΩ resistor ──→ 3.3V Rail

Encoder GND pin ──→ GND Rail
```

**Repeat for additional encoders:**
- Encoder 2: D3, D4, D5
- Encoder 3: D6, D7, D8 (next chip)
- Etc.

**Critical:** Pull-ups must go to **3.3V**, not 5V!

### Step 5: Upload Test Firmware

**Create Arduino sketch:**
```cpp
// File: ShiftRegisterTest.ino
// Quick test of 74HC165 breakout board @ 3.3V

#define LATCH_PIN 27  // GPIO27 (Pin 11 on 38-pin)
#define CLK_PIN   14  // GPIO14 (Pin 12 on 38-pin)
#define DATA_PIN  12  // GPIO12 (Pin 13 on 38-pin)

void setup() {
  Serial.begin(115200);

  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);

  digitalWrite(LATCH_PIN, HIGH);
  digitalWrite(CLK_PIN, LOW);

  Serial.println("74HC165 @ 3.3V Test");
  Serial.println("Rotate encoders and watch output");
}

void loop() {
  // Latch parallel data
  digitalWrite(LATCH_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(5);

  // Read shift register chain
  uint8_t numChips = 4;  // Adjust to your board (3 or 4 chips)
  uint8_t data[numChips];

  for (int chip = 0; chip < numChips; chip++) {
    data[chip] = 0;
    for (int bit = 0; bit < 8; bit++) {
      data[chip] |= (digitalRead(DATA_PIN) << bit);

      // Clock pulse
      digitalWrite(CLK_PIN, HIGH);
      delayMicroseconds(5);
      digitalWrite(CLK_PIN, LOW);
      delayMicroseconds(5);
    }
  }

  // Print binary representation
  Serial.print("Chips: ");
  for (int i = 0; i < numChips; i++) {
    for (int bit = 7; bit >= 0; bit--) {
      Serial.print((data[i] >> bit) & 1);
    }
    Serial.print(" ");
  }
  Serial.println();

  delay(100);  // 10Hz update rate for testing
}
```

**Upload and test:**
```
1. Open Serial Monitor (115200 baud)
2. Should see stream of binary data
3. Rotate encoder → bits should toggle
4. Press encoder button → bit should change
5. If all 1s → encoders floating (need pull-ups!)
6. If all 0s → encoders shorted or wrong voltage
```

### Step 6: Validate Performance

**Test checklist:**
- [ ] All encoder inputs read correctly
- [ ] Rotating encoder shows CLK/DT pattern (01, 00, 10, 11 sequence)
- [ ] Button presses detected (bit goes LOW)
- [ ] No false triggers when idle
- [ ] Fast encoder rotation doesn't miss steps
- [ ] Board runs cool (no overheating)
- [ ] 3.3V rail stable under load

**Measure scan rate:**
Add to test sketch:
```cpp
unsigned long lastPrint = 0;
unsigned long scanCount = 0;

void loop() {
  // ... existing scan code ...
  scanCount++;

  if (millis() - lastPrint >= 1000) {
    Serial.print("Scan rate: ");
    Serial.print(scanCount);
    Serial.println(" Hz");
    scanCount = 0;
    lastPrint = millis();
  }
}
```

**Expected:** 1000-5000 Hz scan rate (even with delays for testing)

---

## Phase 1 Success Criteria

Before moving to PCB design, verify:

✅ **Power System Works:**
- 3.3V regulator outputs 3.2-3.4V stable
- No voltage drop under load
- Board runs cool

✅ **Shift Registers Work @ 3.3V:**
- All inputs read correctly
- Data chain shifts properly
- No intermittent failures

✅ **Encoders Work:**
- Quadrature decoding works
- No missed steps during fast rotation
- Button presses detected
- Pull-ups to 3.3V sufficient

✅ **Performance Acceptable:**
- Scan rate: 1-5 kHz minimum
- No false triggers
- Reliable across temperature range

**If all checks pass → Proceed to Phase 2 (PCB design)**
**If issues found → Troubleshoot using guide below**

---

## Phase 1 Troubleshooting

### Problem: All inputs read as HIGH (1)

**Cause:** Missing or incorrect pull-ups
**Solution:**
- Verify 10kΩ resistors from encoder pins to 3.3V rail
- Check continuity with multimeter
- Ensure pull-ups go to 3.3V, NOT 5V

### Problem: All inputs read as LOW (0)

**Cause:** Encoders shorted or wrong VCC voltage
**Solution:**
- Check shift register board VCC = 3.3V (not 5V!)
- Verify encoder GND connections
- Check for solder bridges on breakout board

### Problem: Intermittent/erratic readings

**Cause:** Voltage drop, noise, or poor connections
**Solution:**
- Add 100nF ceramic cap near shift register VCC/GND
- Check all jumper wire connections (reseat firmly)
- Verify 3.3V rail voltage under load
- Shorten wire lengths if possible

### Problem: ESP32 won't program

**Cause:** Wrong USB cable, driver issue, or boot mode
**Solution:**
- Use data USB cable (not charge-only)
- Install CP2102 or CH340 drivers
- Hold BOOT button while uploading
- Try different USB port

### Problem: Low scan rate (<500 Hz)

**Cause:** Delays in code or slow clock speed
**Solution:**
- Remove Serial.print statements from loop
- Reduce delay() to 1ms or remove
- Use delayMicroseconds() instead of delay()
- Check SPI clock speed (should be 1 MHz)

---

## Phase 2: Design Your First PCB

Once breadboard works perfectly, move to PCB design.

### Recommended Approach

**Option A: Simple Breakout Adapter PCB** (Fastest)
- Small PCB (~100mm × 80mm)
- Just holds ESP32, regulator, and mounting for breakout boards
- Wire encoders with ribbon cables
- Good for testing full system before committing to large panel

**Option B: Full Panel PCB** (Follows guide exactly)
- 300mm × 120mm PCB with all 32 encoders
- Breakout boards mounted on PCB
- Everything integrated
- See `PCB-Builders-Guide-Alternative-Breakout-Boards.md`

### Next Steps

1. **Choose KiCad project template:**
   - Open KiCad 9
   - New Project → "MIDI_Kraken_Test_Panel"
   - Save in `hardware/` directory

2. **Create schematic:**
   - Add ESP32 38-pin symbol
   - Add LM1117-3.3 regulator
   - Add LM7805 regulator (for ESP32)
   - Add breakout board symbols (custom or generic headers)
   - Add encoder symbols
   - Add all connections per guides

3. **Layout PCB:**
   - Set board size
   - Place components
   - Route power (3.3V + 5V + GND)
   - Route control signals (CLK, LATCH, DATA)
   - Route data chain between boards
   - Add copper pour (GND plane)

4. **Generate Gerbers:**
   - Run DRC (Design Rule Check)
   - Fix all errors
   - Generate Gerber files
   - Create BOM

5. **Order:**
   - Upload Gerbers to JLCPCB or PCBWay
   - Order components from DigiKey/Mouser
   - Wait 1-2 weeks for delivery

---

## Shopping List (Phase 1 Prototype)

### Immediate Needs (Breadboard Test)

**From Your Inventory:**
- [x] ESP32-S 38-pin board
- [x] 74HC165 breakout boards (4-chip and/or 3-chip)

**Need to Order:**
```
Electronics Distributor (DigiKey, Mouser, Amazon):
[ ] LM1117-3.3 regulator (SOT-223 or TO-220) - Qty: 1
[ ] 10µF electrolytic caps (16V) - Qty: 2
[ ] 100nF ceramic caps (50V) - Qty: 2
[ ] 10kΩ resistors (1/4W) - Qty: 20 (for 4 encoders)
[ ] 4.7kΩ resistors (1/4W) - Qty: 2 (I2C pull-ups)
[ ] Breadboard (830 tie-points) - Qty: 1
[ ] Jumper wire kit (male-male, male-female) - Qty: 1 kit
[ ] 5V power supply (2A minimum) - Qty: 1

Encoders (Amazon, AliExpress, or electronics store):
[ ] KY-050 rotary encoders - Qty: 4-8 (test quantity)
    Alternative: Any quadrature encoder with push button
```

**Estimated cost for prototype: ~$15-25**

### Phase 2 Needs (Full Panel)

**PCB Fabrication:**
- 1× PCB (300×120mm, 2-layer) - ~$10-15 for 5 boards

**Components (per panel):**
- 32× KY-050 encoders - ~$16 (bulk)
- 100× 10kΩ resistors - ~$2
- Power components - ~$5
- Connectors and misc - ~$5
- **Total: ~$38-43 per panel**

---

## Timeline Estimate

**Phase 1 (Breadboard Prototype):**
- Order components: 3-5 days shipping
- Assembly: 2-4 hours
- Testing: 1-2 hours
- Firmware development: 2-4 hours
- **Total: 1 week (mostly waiting for parts)**

**Phase 2 (First PCB Panel):**
- KiCad design: 8-12 hours (first time)
- PCB fabrication: 5-7 days (JLCPCB)
- Component sourcing: 3-5 days
- Assembly: 4-6 hours
- Testing: 2-3 hours
- **Total: 2-3 weeks**

**Phase 3 (Remaining 7 Panels):**
- PCB fabrication: Order all 7 at once with first batch
- Assembly: 4-6 hours each × 7 = 28-42 hours
- Testing: 2-3 hours each × 7 = 14-21 hours
- **Total: 1-2 months (evenings/weekends)**

---

## Success Tips

1. **Start Small:** Breadboard first, PCB second
2. **Test Early:** Don't order 8 panels until one works perfectly
3. **Document Everything:** Take photos, notes, measurements
4. **Ask Questions:** Use Discord, forums, or GitHub issues
5. **Iterate:** First panel will have issues - that's normal
6. **Enjoy the Process:** This is a marathon, not a sprint

---

## When You're Stuck

**Resources:**
- `PCB-Builders-Guide-Alternative-Breakout-Boards.md` - Your build guide
- `ESP32-38Pin-Pinout-Reference.md` - Your board's pinout
- `I2C-Address-Reference.md` - When you get to multi-panel
- Arduino ESP32 forum: https://www.arduino.cc/
- ESP32 subreddit: r/esp32

**Common Issues:**
- 90% of problems are loose wires or wrong voltage
- 9% are incorrect pull-ups (value or rail voltage)
- 1% are actual component failures

**Debug Process:**
1. Check power (5V and 3.3V rails with multimeter)
2. Check connections (continuity test all signals)
3. Check orientation (IC notches, polarity)
4. Check firmware (Serial.print everything)
5. Check assumptions (measure, don't assume)

---

## Ready? Let's Build!

**Your next action:**
```
1. Order components for Phase 1 prototype
2. While waiting, study the guides
3. Set up Arduino IDE + ESP32 support
4. Review breadboard layout in this guide
5. Gather your tools (multimeter, wire strippers, etc.)
```

**When parts arrive:**
```
Day 1: Build 3.3V power supply, test with multimeter
Day 2: Connect ESP32, upload Blink test
Day 3: Connect one breakout board, test shift register
Day 4: Connect encoders, upload test firmware
Day 5: Validate and document results
```

**You've got this!** 🎛️

Hit me up when your parts arrive and we'll work through Phase 1 together.
