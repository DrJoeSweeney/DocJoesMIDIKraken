#ifndef MIDI_ENGINE_H
#define MIDI_ENGINE_H

#include <Arduino.h>
#include <Protocol.h>

#ifdef ARDUINO_TEENSY40
#include <usb_midi.h>
#endif

/**
 * MIDIEngine - MIDI Message Generation and Transmission
 *
 * Generates and sends MIDI messages for all control types.
 * Supports 7-bit and 14-bit MIDI, 4 virtual devices, and all standard messages.
 *
 * Features:
 * - 4 virtual USB MIDI devices
 * - 7-bit standard MIDI (CC 0-127)
 * - 14-bit high-resolution MIDI (CC pairs)
 * - Pitch bend (14-bit)
 * - Program change
 * - Message throttling (2,500 msg/sec)
 *
 * Typical usage:
 *   MIDIEngine midi;
 *   midi.begin();
 *   midi.sendCC(virtualDevice, channel, ccNumber, value);
 *   midi.sendCC14bit(virtualDevice, channel, ccNumber, value14bit);
 *   midi.sendPitchBend(virtualDevice, channel, value14bit);
 */
class MIDIEngine {
public:
    MIDIEngine();

    /**
     * Initialize MIDI engine
     */
    void begin();

    /**
     * Send 7-bit Control Change
     * @param device - Virtual device (0-3)
     * @param channel - MIDI channel (0-15)
     * @param ccNumber - CC number (0-127)
     * @param value - CC value (0-127)
     * @return true if sent successfully
     */
    bool sendCC(uint8_t device, uint8_t channel, uint8_t ccNumber, uint8_t value);

    /**
     * Send 14-bit Control Change (MSB + LSB pair)
     * @param device - Virtual device (0-3)
     * @param channel - MIDI channel (0-15)
     * @param ccNumber - CC number (0-31, uses ccNumber and ccNumber+32)
     * @param value14 - 14-bit value (0-16383)
     * @return true if sent successfully
     */
    bool sendCC14bit(uint8_t device, uint8_t channel, uint8_t ccNumber, uint16_t value14);

    /**
     * Send Pitch Bend
     * @param device - Virtual device (0-3)
     * @param channel - MIDI channel (0-15)
     * @param value14 - 14-bit value (0-16383, center=8192)
     * @return true if sent successfully
     */
    bool sendPitchBend(uint8_t device, uint8_t channel, uint16_t value14);

    /**
     * Send Program Change
     * @param device - Virtual device (0-3)
     * @param channel - MIDI channel (0-15)
     * @param program - Program number (0-127)
     * @return true if sent successfully
     */
    bool sendProgramChange(uint8_t device, uint8_t channel, uint8_t program);

    /**
     * Send Note On
     * @param device - Virtual device (0-3)
     * @param channel - MIDI channel (0-15)
     * @param note - Note number (0-127)
     * @param velocity - Velocity (1-127, 0 = Note Off)
     * @return true if sent successfully
     */
    bool sendNoteOn(uint8_t device, uint8_t channel, uint8_t note, uint8_t velocity);

    /**
     * Send Note Off
     * @param device - Virtual device (0-3)
     * @param channel - MIDI channel (0-15)
     * @param note - Note number (0-127)
     * @return true if sent successfully
     */
    bool sendNoteOff(uint8_t device, uint8_t channel, uint8_t note);

    /**
     * Send All Notes Off (panic)
     * @param device - Virtual device (0-3)
     * @param channel - MIDI channel (0-15, or 255 for all channels)
     * @return true if sent successfully
     */
    bool sendAllNotesOff(uint8_t device, uint8_t channel = 255);

    /**
     * Process control event and send appropriate MIDI message
     * @param config - Control configuration
     * @param value - Control value
     * @return true if MIDI sent
     */
    bool processControl(const ControlConfig& config, uint8_t value);

    /**
     * Get MIDI message statistics
     */
    uint32_t getMessagesSent() const { return m_messagesSent; }
    uint32_t getMessagesDropped() const { return m_messagesDropped; }
    float getMessageRate() const;  // Messages per second

private:
    uint32_t m_messagesSent;
    uint32_t m_messagesDropped;
    uint32_t m_lastRateCalcTime;
    uint32_t m_messagesInPeriod;

    // Throttling (max 2,500 messages/second)
    static const uint32_t MAX_MESSAGES_PER_SECOND = 2500;
    static const uint32_t MIN_MESSAGE_INTERVAL_US = 400;  // 1000000 / 2500
    uint32_t m_lastMessageTime;

    bool shouldThrottle();
    void recordMessage();

#ifdef ARDUINO_TEENSY40
    // Teensy-specific MIDI sending
    void sendMIDIMessage(uint8_t device, uint8_t status, uint8_t data1, uint8_t data2);
    void sendMIDIMessage(uint8_t device, uint8_t status, uint8_t data1);
#endif
};

#endif // MIDI_ENGINE_H
