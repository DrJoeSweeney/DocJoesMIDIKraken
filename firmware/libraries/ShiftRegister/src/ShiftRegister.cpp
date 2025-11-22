#include "ShiftRegister.h"

ShiftRegister::ShiftRegister(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t numChips)
    : m_dataPin(dataPin)
    , m_clockPin(clockPin)
    , m_latchPin(latchPin)
    , m_numChips(numChips)
    , m_buffer(nullptr)
    , m_lastReadTime(0)
{
    m_buffer = new uint8_t[m_numChips];
    memset(m_buffer, 0, m_numChips);
}

void ShiftRegister::begin() {
    pinMode(m_dataPin, INPUT);
    pinMode(m_clockPin, OUTPUT);
    pinMode(m_latchPin, OUTPUT);

    digitalWrite(m_clockPin, LOW);
    digitalWrite(m_latchPin, HIGH);
}

bool ShiftRegister::read() {
    uint32_t startTime = micros();

    // Latch current state into shift registers
    latch();

    // Read all bytes
    for (int i = m_numChips - 1; i >= 0; i--) {
        uint8_t byte = 0;

        // Read 8 bits (MSB first)
        for (int bit = 7; bit >= 0; bit--) {
            if (digitalRead(m_dataPin) == HIGH) {
                byte |= (1 << bit);
            }
            clockPulse();
        }

        m_buffer[i] = byte;
    }

    m_lastReadTime = micros() - startTime;
    return true;
}

uint8_t ShiftRegister::getByte(uint8_t byteIndex) {
    if (byteIndex >= m_numChips) {
        return 0;
    }
    return m_buffer[byteIndex];
}

bool ShiftRegister::getBit(uint16_t bitIndex) {
    uint8_t byteIndex = bitIndex / 8;
    uint8_t bitOffset = bitIndex % 8;

    if (byteIndex >= m_numChips) {
        return false;
    }

    return (m_buffer[byteIndex] & (1 << bitOffset)) != 0;
}

const uint8_t* ShiftRegister::getBuffer() const {
    return m_buffer;
}

void ShiftRegister::latch() {
    // Pulse latch LOW to load parallel inputs
    digitalWrite(m_latchPin, LOW);
    delayMicroseconds(1);
    digitalWrite(m_latchPin, HIGH);
    delayMicroseconds(1);
}

void ShiftRegister::clockPulse() {
    // Pulse clock HIGH to shift data
    digitalWrite(m_clockPin, HIGH);
    delayMicroseconds(1);
    digitalWrite(m_clockPin, LOW);
    delayMicroseconds(1);
}
