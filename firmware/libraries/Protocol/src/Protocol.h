#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

// ============================================================================
// SYSTEM CONSTANTS
// ============================================================================

#define TOTAL_CONTROLS 619
#define NUM_ENCODERS 284
#define NUM_ENCODER_BUTTONS 284
#define NUM_STANDALONE_BUTTONS 32
#define NUM_SNAPSHOT_BUTTONS 19
#define NUM_TOTAL_BUTTONS (NUM_ENCODER_BUTTONS + NUM_STANDALONE_BUTTONS + NUM_SNAPSHOT_BUTTONS)

#define NUM_SESSIONS 128
#define NUM_SNAPSHOTS 16
#define NUM_VIRTUAL_DEVICES 4
#define NUM_MIDI_CHANNELS 16

#define SESSION_FILE_SIZE 103424  // ~101KB per session
#define TOTAL_STORAGE_SIZE 13238272  // ~13MB for 128 sessions

// ============================================================================
// CONTROL TYPES & MODES
// ============================================================================

enum ControlType : uint8_t {
    CONTROL_ENCODER = 0,
    CONTROL_BUTTON = 1,
    CONTROL_ENCODER_BUTTON = 2,
    CONTROL_JOYSTICK_X = 3,
    CONTROL_JOYSTICK_Y = 4,
    CONTROL_JOYSTICK_BUTTON = 5
};

enum EncoderMode : uint8_t {
    ENCODER_ABSOLUTE = 0,      // 0-127 with wrapping
    ENCODER_RELATIVE_1 = 1,    // 64 = center, <64 = CCW, >64 = CW
    ENCODER_RELATIVE_2 = 2,    // 63/65 for slow, 0/127 for fast
    ENCODER_RELATIVE_3 = 3     // Two's complement relative
};

enum ButtonAction : uint8_t {
    BUTTON_CC_TOGGLE = 0,      // Toggle between 0 and 127
    BUTTON_CC_MOMENTARY = 1,   // 127 when pressed, 0 when released
    BUTTON_CC_VALUE = 2,       // Send specific value
    BUTTON_PROGRAM_CHANGE = 3, // MIDI Program Change
    BUTTON_BANK_SWITCH = 4,    // Switch Bank A/B
    BUTTON_SNAPSHOT_RECALL = 5,// Recall snapshot
    BUTTON_SNAPSHOT_SAVE = 6,  // Save snapshot
    BUTTON_PANIC = 7           // All notes off
};

// ============================================================================
// TRANSITION TYPES
// ============================================================================

enum TransitionType : uint8_t {
    TRANSITION_INSTANT = 0,
    TRANSITION_LINEAR = 1,
    TRANSITION_EASE_IN = 2,
    TRANSITION_EASE_OUT = 3,
    TRANSITION_EASE_IN_OUT = 4,
    TRANSITION_EXPONENTIAL = 5,
    TRANSITION_STEPPED = 6
};

enum TransitionTimebase : uint8_t {
    TIMEBASE_MILLISECONDS = 0,
    TIMEBASE_MIDI_CLOCK = 1
};

// Musical time values (in MIDI clock ticks, 24 PPQN)
enum MusicalTime : uint16_t {
    MUSICAL_TIME_1_16TH = 6,     // 1/16 note
    MUSICAL_TIME_1_8TH = 12,     // 1/8 note
    MUSICAL_TIME_1_4TH = 24,     // 1/4 note
    MUSICAL_TIME_1_2 = 48,       // 1/2 note
    MUSICAL_TIME_1_BAR = 96,     // 1 bar (4/4 time)
    MUSICAL_TIME_2_BARS = 192,   // 2 bars
    MUSICAL_TIME_4_BARS = 384,   // 4 bars
    MUSICAL_TIME_8_BARS = 768    // 8 bars
};

// ============================================================================
// BULK ASSIGNMENT MODES
// ============================================================================

enum BulkAssignMode : uint8_t {
    BULK_SEQUENTIAL_GLOBAL = 0,    // CC 0, 1, 2, 3... across all controls
    BULK_SEQUENTIAL_PER_PANEL = 1, // CC 0-N per panel, resets each panel
    BULK_BY_PANEL_OFFSET = 2,      // Panel 1: CC 0-63, Panel 2: CC 64-127
    BULK_BY_SECTION_TYPE = 3,      // OSC: CC 0-31, ENV: CC 32-63, etc.
    BULK_BY_VIRTUAL_DEVICE = 4,    // Spread across 4 virtual devices
    BULK_BY_MIDI_CHANNEL = 5,      // Spread across 16 MIDI channels
    BULK_INTERLEAVED_AB = 6,       // Alternating Bank A/B assignments
    BULK_MIRRORED_AB = 7           // Same CC, different devices A/B
};

// ============================================================================
// EVENT MESSAGE (I2C Communication)
// ============================================================================

#pragma pack(push, 1)

struct EventMessage {
    uint16_t globalID;      // 0-618
    uint8_t value;          // 0-127 for MIDI value
    uint8_t flags;          // Event flags (button press/release, etc.)
    uint32_t timestamp;     // Microseconds (for diagnostics)
};

// Event flags
#define EVENT_FLAG_BUTTON_PRESSED  0x01
#define EVENT_FLAG_BUTTON_RELEASED 0x02
#define EVENT_FLAG_ENCODER_CW      0x04
#define EVENT_FLAG_ENCODER_CCW     0x08
#define EVENT_FLAG_PRIORITY        0x80

#pragma pack(pop)

// ============================================================================
// CONTROL CONFIGURATION (48 bytes)
// ============================================================================

#pragma pack(push, 1)

struct ControlConfig {
    // Identity (4 bytes)
    uint16_t globalID;           // 0-618
    ControlType controlType;     // ENCODER, BUTTON, etc.
    uint8_t flags;               // Control flags

    // Snapshot behavior (4 bytes)
    uint8_t snapshotFlags;       // Recall behavior flags
    uint8_t reserved1;
    uint16_t reserved2;

    // MIDI Bank A (4 bytes)
    uint8_t ccNumber;            // 0-127
    uint8_t midiChannel;         // 0-15
    uint8_t virtualDevice;       // 0-3
    uint8_t resolution;          // 0=7-bit, 1=14-bit

    // Value range (4 bytes)
    uint8_t minValue;            // 0-127
    uint8_t maxValue;            // 0-127
    uint8_t defaultValue;        // 0-127
    uint8_t currentBank;         // 0=A, 1=B

    // Encoder-specific (4 bytes)
    EncoderMode encoderMode;     // Absolute/Relative modes
    uint8_t acceleration;        // 0-255 acceleration curve
    uint8_t threshold;           // Minimum delta before sending
    int8_t deadZone;             // Dead zone for joysticks

    // Button-specific (4 bytes)
    ButtonAction buttonAction;   // Toggle, momentary, etc.
    uint8_t buttonValue;         // Value to send (for CC_VALUE mode)
    uint8_t buttonToggleState;   // Current toggle state (0 or 1)
    uint8_t reserved3;

    // MIDI Bank B (4 bytes)
    uint8_t ccNumberB;           // Bank B CC
    uint8_t midiChannelB;        // Bank B channel
    uint8_t virtualDeviceB;      // Bank B device
    uint8_t resolutionB;         // Bank B resolution

    // Organization (4 bytes)
    uint8_t panelID;             // Which panel (0-15)
    uint8_t groupID;             // Control group (0-255)
    uint8_t groupFlags;          // Group behavior flags
    uint8_t reserved4;

    // Label (16 bytes)
    char label[16];              // Human-readable name

    // Diagnostics (8 bytes)
    uint32_t eventCount;         // Total events sent
    uint32_t lastEventTime;      // Last event timestamp (ms)
};

// Control flags
#define CONTROL_FLAG_ENABLED       0x01
#define CONTROL_FLAG_INVERTED      0x02
#define CONTROL_FLAG_14BIT         0x04
#define CONTROL_FLAG_SEND_ON_LOAD  0x08

// Snapshot flags
#define SNAPSHOT_FLAG_ZEROED       0x01  // Excluded from recall
#define SNAPSHOT_FLAG_PANEL_ZERO   0x02  // Panel excluded
#define SNAPSHOT_FLAG_BANK_ZERO    0x04  // Bank excluded
#define SNAPSHOT_FLAG_GLOBAL_ZERO  0x08  // All excluded

// Group flags
#define GROUP_FLAG_LINKED          0x01  // Linked controls move together
#define GROUP_FLAG_INVERSE_LINKED  0x02  // Linked but inverted

#pragma pack(pop)

// ============================================================================
// CONTROL STATE (4 bytes)
// ============================================================================

#pragma pack(push, 1)

struct ControlState {
    uint8_t value;               // Current MIDI value (0-127)
    uint8_t lastSentValue;       // Last sent value (for change detection)
    uint8_t targetValue;         // Target value (for morphing)
    uint8_t stateFlags;          // Runtime state flags
};

// State flags
#define STATE_FLAG_TRANSITIONING   0x01
#define STATE_FLAG_DIRTY           0x02  // Needs to be sent
#define STATE_FLAG_BUTTON_STATE    0x04  // Current button state

#pragma pack(pop)

// ============================================================================
// TRANSITION SETTINGS (12 bytes)
// ============================================================================

#pragma pack(push, 1)

struct TransitionSettings {
    TransitionType type;         // Transition curve type
    TransitionTimebase timebase; // Milliseconds or MIDI clock
    uint16_t duration;           // Duration (ms or MIDI ticks)
    uint8_t stepValue;           // Step size for STEPPED transitions
    uint8_t reserved[7];
};

#pragma pack(pop)

// ============================================================================
// SNAPSHOT (2488 bytes)
// ============================================================================

#pragma pack(push, 1)

struct Snapshot {
    char name[32];                            // Snapshot name
    TransitionSettings transition;            // 12 bytes
    uint8_t values[TOTAL_CONTROLS];           // 619 values
    uint16_t zeroMask[TOTAL_CONTROLS / 16 + 1]; // 40 bytes bitmask
    uint32_t timestamp;                       // Last save time
    uint32_t crc32;                           // Data integrity
    uint8_t reserved[1781];                   // Padding to 2488 bytes
};

#pragma pack(pop)

// ============================================================================
// SESSION FILE (103424 bytes)
// ============================================================================

#pragma pack(push, 1)

struct SessionFile {
    char name[64];                            // Session name
    uint8_t version;                          // File format version
    uint8_t activeSnapshot;                   // Currently active (0-15)
    uint8_t reserved[62];                     // Reserved

    ControlConfig controls[TOTAL_CONTROLS];   // 619 × 48 = 29,712 bytes
    Snapshot snapshots[NUM_SNAPSHOTS];        // 16 × 2488 = 39,808 bytes

    uint32_t globalZeroMask;                  // Global zero function state
    uint32_t panelZeroMask;                   // Panel zero states
    uint32_t bankZeroMask;                    // Bank zero states

    uint32_t createdTime;                     // Unix timestamp
    uint32_t modifiedTime;                    // Unix timestamp
    uint32_t crc32;                           // File integrity

    uint8_t padding[33792];                   // Pad to 103424 bytes
};

#pragma pack(pop)

// ============================================================================
// SYSTEM STATUS
// ============================================================================

struct SystemStatus {
    bool i2cHealthy[9];           // Health of 9 I2C ESP32s (8 synth + 1 FX)
    uint32_t scanRate[9];         // Scan rate per ESP32 (Hz)
    uint32_t dropRate[9];         // Dropped events per ESP32
    uint32_t midiMessagesSent;    // Total MIDI messages sent
    uint32_t uptime;              // System uptime (seconds)
    float cpuUsage;               // CPU usage (0-100%)
    uint32_t freeMemory;          // Free memory (bytes)
    bool sdCardPresent;           // SD card detected
    bool wifiConnected;           // WiFi status
};

// ============================================================================
// DIAGNOSTIC METRICS
// ============================================================================

struct DiagnosticMetrics {
    uint32_t scanCycleTime;       // Last scan cycle time (µs)
    uint32_t avgScanCycleTime;    // Average scan cycle (µs)
    uint32_t maxScanCycleTime;    // Peak scan cycle (µs)
    uint32_t i2cLatency;          // I2C transaction time (µs)
    uint32_t queueDepth;          // Event queue depth
    uint32_t eventsProcessed;     // Total events processed
    uint32_t eventsDropped;       // Total events dropped
    float dropRate;               // Drop rate percentage
};

// ============================================================================
// I2C COMMAND CODES
// ============================================================================

enum I2CCommand : uint8_t {
    CMD_GET_EVENTS = 0x01,        // Request events from ESP32
    CMD_SET_CONFIG = 0x02,        // Update control configuration
    CMD_GET_STATUS = 0x03,        // Get ESP32 status
    CMD_RESET = 0x04,             // Reset ESP32
    CMD_ENABLE_CONTROLS = 0x05,   // Enable/disable controls
    CMD_SET_LED = 0x06,           // Set status LED
    CMD_DIAGNOSTICS = 0x07,       // Request diagnostics
    CMD_PING = 0x08               // Ping for health check
};

// ============================================================================
// WEB API MESSAGE TYPES
// ============================================================================

enum WebMessageType : uint8_t {
    WEB_MSG_SESSION_LIST = 0x10,
    WEB_MSG_SESSION_LOAD = 0x11,
    WEB_MSG_SESSION_SAVE = 0x12,
    WEB_MSG_SESSION_DELETE = 0x13,
    WEB_MSG_CONTROL_UPDATE = 0x20,
    WEB_MSG_SNAPSHOT_RECALL = 0x21,
    WEB_MSG_SNAPSHOT_SAVE = 0x22,
    WEB_MSG_BULK_ASSIGN = 0x30,
    WEB_MSG_SYSTEM_STATUS = 0x40,
    WEB_MSG_REALTIME_EVENT = 0x50
};

#endif // PROTOCOL_H
