# MIDI Kraken - DocJoesMIDIKraken

A professional-grade, highly configurable MIDI controller with 620+ controls, web-based configuration, and comprehensive session management.

## 📚 Documentation

### **Primary Documents** (START HERE)

1. **[MIDI Kraken - Complete Software Specification.md](./MIDI%20Kraken%20-%20Complete%20Software%20Specification.md)**
   - **THE** comprehensive specification (65 pages)
   - Complete hardware architecture
   - Full software design
   - All features documented
   - Ready for implementation

2. **[CHANGELOG.md](./CHANGELOG.md)**
   - Complete project evolution history
   - All design decisions documented
   - Performance milestones
   - Version history

## 🎯 Quick Stats

- **620+ controls**: 284 encoders + 316 buttons + 1 joystick
- **<500µs latency**: Interrupt-driven communication
- **128 sessions**: Full configuration + 16 snapshots each
- **Web configurable**: Phone, tablet, or laptop
- **4 virtual MIDI devices**: Up to 64 MIDI channels
- **~$385 cost**: Exceptional value

## 🏗️ System Overview

```
Hardware:
├─ 7 × ESP32 (peripheral processors + WiFi)
├─ 1 × Teensy 4.0 (main controller)
├─ 284 × KY-050 rotary encoders (with buttons)
├─ 51 × Buttons (standalone + snapshot panel)
├─ 1 × Analog joystick (2-axis + button)
├─ 114 × 74HC165 shift registers
├─ 1 × 4" TFT display
└─ 1 × 16GB SD card

Features:
├─ 128 sessions with 16 snapshots each
├─ Snapshot morphing with transitions
├─ MIDI clock synchronization
├─ Web-based configuration
├─ JSON import/export
├─ 8 bulk CC assignment modes
├─ Zero functions (selective recall)
├─ 7-bit and 14-bit MIDI
├─ Bank A/B (double CC mappings)
├─ MIDI learn mode
├─ Auto-save
└─ Real-time monitoring
```

## 🚀 Key Features

### Control Surface
- 284 rotary encoders with acceleration curves
- 284 encoder push buttons (configurable actions)
- 32 standalone buttons
- 19 snapshot recall buttons
- 1 analog joystick (pitch bend + modulation)

### MIDI
- 4 virtual MIDI devices
- 16 MIDI channels per device
- 7-bit standard MIDI (0-127)
- 14-bit high-resolution MIDI (0-16383)
- MIDI 2.0 ready (optional future)
- Pitch bend (14-bit)
- 2,500 messages/second throughput

### Sessions & Snapshots
- 128 named sessions
- 16 snapshots per session
- Snapshot morphing with transitions
- Musical time transitions (MIDI clock)
- Multiple interpolation curves
- Zero functions (selective recall)
- JSON import/export

### Configuration
- Web-based (phone/tablet/laptop)
- Onboard 4" TFT display
- 8 bulk CC assignment modes
- MIDI learn mode
- Bank A/B switching
- Control grouping
- Panel organization

### Performance
- <500µs average latency
- 2-3kHz scan rate per section
- Dual-core ESP32 processing
- DMA shift register reading
- Event batching
- <0.01% dropped events

## 🔧 Development Status

**Design Phase:** ✅ Complete
**Implementation Phase:** 🚧 Ready to Start
**Estimated Build Time:** 6-7 months part-time

## 📋 Next Steps

1. Order components (~$385)
2. Design PCBs for panels
3. Implement ESP32 firmware
4. Implement Teensy firmware
5. Develop web interface
6. Build enclosure
7. Final integration & testing

## 💰 Cost Breakdown

| Component | Quantity | Cost |
|-----------|----------|------|
| ESP32 modules | 7 | $35 |
| Teensy 4.0 | 1 | $30 |
| KY-050 encoders | 284 | $142 |
| Buttons | 51 | $5 |
| 74HC165 shift registers | 114 | $57 |
| Joystick | 1 | $3 |
| 4" TFT display | 1 | $15 |
| 16GB SD card | 1 | $8 |
| Power supply (5V 3A) | 1 | $10 |
| PCBs (custom) | 5 | $40 |
| Enclosure | 1 | $20 |
| Misc (wire, connectors) | - | $20 |
| **Total** | | **~$385** |

## 🏛️ Architecture Highlights

### Distributed Processing
- 7 ESP32s handling peripheral I/O
- Teensy 4.0 as main MIDI controller
- Parallel scanning: 2-3kHz per section
- Interrupt-driven communication

### Multi-Bus I2C
- 3 parallel I2C buses (1MHz Fast Mode+)
- Bus 0: ESP32 #1, #2
- Bus 1: ESP32 #3, #4
- Bus 2: ESP32 #5, #6, #7

### Dual-Core ESP32
- Core 0: High-priority scanning (3kHz)
- Core 1: Communication & management
- Lock-free queue between cores
- DMA for shift register reads

### WiFi Configuration
- ESP32 #7 with WiFi
- Full web-based configuration
- REST API
- WebSocket real-time updates
- 128-session storage on SD card
- JSON import/export

## 📖 Documentation Structure

The complete specification covers:

1. **System Architecture** - Hardware overview, component specs
2. **Communication & I/O** - I2C, UART, shift registers
3. **Data Structures** - All config and state structures
4. **Core Firmware** - ESP32 and Teensy firmware
5. **Libraries** - 24 reusable libraries documented
6. **Features** - Snapshots, sessions, zero functions, etc.
7. **User Interfaces** - Web, REST API, onboard display
8. **Development** - Build system, testing, deployment

## 🎓 Design Philosophy

- **Performance First**: Low latency, high scan rates
- **Modular**: Build and test incrementally
- **User-Friendly**: Web + onboard UI
- **Expandable**: Easy to add features
- **Affordable**: ~$385 for 620 controls

## 📞 Support

For questions about this specification or implementation guidance, refer to:
- **Complete Software Specification.md** - All technical details
- **CHANGELOG.md** - Design decisions and evolution

## 📄 License

[Your License Here]

---

**Version:** 2.0 Final
**Last Updated:** 2025-01-22
**Status:** Design Complete, Ready for Implementation
**Project:** DocJoesMIDIKraken
