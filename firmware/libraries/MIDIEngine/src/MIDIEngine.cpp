#include "MIDIEngine.h"

MIDIEngine::MIDIEngine()
    : m_messagesSent(0)
    , m_messagesDropped(0)
    , m_lastRateCalcTime(0)
    , m_messagesInPeriod(0)
    , m_lastMessageTime(0)
{
}

void MIDIEngine::begin() {
    m_messagesSent = 0;
    m_messagesDropped = 0;
    m_lastRateCalcTime = millis();
    m_lastMessageTime = micros();
}

bool MIDIEngine::sendCC(uint8_t device, uint8_t channel, uint8_t ccNumber, uint8_t value) {
    if (shouldThrottle()) {
        m_messagesDropped++;
        return false;
    }

    uint8_t status = 0xB0 | (channel & 0x0F);  // Control Change
#ifdef ARDUINO_TEENSY40
    sendMIDIMessage(device, status, ccNumber, value);
#endif

    recordMessage();
    return true;
}

bool MIDIEngine::sendCC14bit(uint8_t device, uint8_t channel, uint8_t ccNumber, uint16_t value14) {
    if (ccNumber >= 32) {
        return false;  // Only CC 0-31 support 14-bit
    }

    if (shouldThrottle()) {
        m_messagesDropped++;
        return false;
    }

    // Send MSB (CC 0-31)
    uint8_t msb = (value14 >> 7) & 0x7F;
    uint8_t lsb = value14 & 0x7F;

    uint8_t status = 0xB0 | (channel & 0x0F);

#ifdef ARDUINO_TEENSY40
    sendMIDIMessage(device, status, ccNumber, msb);
    sendMIDIMessage(device, status, ccNumber + 32, lsb);  // LSB is +32
#endif

    recordMessage();
    recordMessage();  // Two messages sent
    return true;
}

bool MIDIEngine::sendPitchBend(uint8_t device, uint8_t channel, uint16_t value14) {
    if (shouldThrottle()) {
        m_messagesDropped++;
        return false;
    }

    uint8_t status = 0xE0 | (channel & 0x0F);  // Pitch Bend
    uint8_t lsb = value14 & 0x7F;
    uint8_t msb = (value14 >> 7) & 0x7F;

#ifdef ARDUINO_TEENSY40
    sendMIDIMessage(device, status, lsb, msb);
#endif

    recordMessage();
    return true;
}

bool MIDIEngine::sendProgramChange(uint8_t device, uint8_t channel, uint8_t program) {
    if (shouldThrottle()) {
        m_messagesDropped++;
        return false;
    }

    uint8_t status = 0xC0 | (channel & 0x0F);  // Program Change

#ifdef ARDUINO_TEENSY40
    sendMIDIMessage(device, status, program);
#endif

    recordMessage();
    return true;
}

bool MIDIEngine::sendNoteOn(uint8_t device, uint8_t channel, uint8_t note, uint8_t velocity) {
    if (shouldThrottle()) {
        m_messagesDropped++;
        return false;
    }

    uint8_t status = 0x90 | (channel & 0x0F);  // Note On

#ifdef ARDUINO_TEENSY40
    sendMIDIMessage(device, status, note, velocity);
#endif

    recordMessage();
    return true;
}

bool MIDIEngine::sendNoteOff(uint8_t device, uint8_t channel, uint8_t note) {
    if (shouldThrottle()) {
        m_messagesDropped++;
        return false;
    }

    uint8_t status = 0x80 | (channel & 0x0F);  // Note Off

#ifdef ARDUINO_TEENSY40
    sendMIDIMessage(device, status, note, 0);
#endif

    recordMessage();
    return true;
}

bool MIDIEngine::sendAllNotesOff(uint8_t device, uint8_t channel) {
    bool success = true;

    if (channel == 255) {
        // All channels
        for (uint8_t ch = 0; ch < 16; ch++) {
            success &= sendCC(device, ch, 123, 0);  // CC 123 = All Notes Off
        }
    } else {
        success = sendCC(device, channel, 123, 0);
    }

    return success;
}

bool MIDIEngine::processControl(const ControlConfig& config, uint8_t value) {
    uint8_t device = config.currentBank == 0 ? config.virtualDevice : config.virtualDeviceB;
    uint8_t channel = config.currentBank == 0 ? config.midiChannel : config.midiChannelB;
    uint8_t ccNumber = config.currentBank == 0 ? config.ccNumber : config.ccNumberB;
    uint8_t resolution = config.currentBank == 0 ? config.resolution : config.resolutionB;

    // Apply min/max range
    uint8_t scaledValue = map(value, 0, 127, config.minValue, config.maxValue);

    if (resolution == 1) {
        // 14-bit MIDI
        uint16_t value14 = map(value, 0, 127, 0, 16383);
        return sendCC14bit(device, channel, ccNumber, value14);
    } else {
        // 7-bit MIDI
        return sendCC(device, channel, ccNumber, scaledValue);
    }
}

float MIDIEngine::getMessageRate() const {
    uint32_t elapsed = millis() - m_lastRateCalcTime;
    if (elapsed == 0) {
        return 0.0f;
    }

    return (m_messagesInPeriod * 1000.0f) / elapsed;
}

bool MIDIEngine::shouldThrottle() {
    uint32_t currentTime = micros();
    uint32_t elapsed = currentTime - m_lastMessageTime;

    return elapsed < MIN_MESSAGE_INTERVAL_US;
}

void MIDIEngine::recordMessage() {
    m_messagesSent++;
    m_messagesInPeriod++;
    m_lastMessageTime = micros();

    // Reset rate calculation every second
    if (millis() - m_lastRateCalcTime >= 1000) {
        m_lastRateCalcTime = millis();
        m_messagesInPeriod = 0;
    }
}

#ifdef ARDUINO_TEENSY40
void MIDIEngine::sendMIDIMessage(uint8_t device, uint8_t status, uint8_t data1, uint8_t data2) {
    // Teensy USB MIDI uses cable number 0-15, we use device number to route
    usbMIDI.send(status, data1, data2, device + 1, 0);  // Cable = device + 1
}

void MIDIEngine::sendMIDIMessage(uint8_t device, uint8_t status, uint8_t data1) {
    usbMIDI.send(status, data1, 0, device + 1, 0);
}
#endif
