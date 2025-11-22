#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include <Arduino.h>

/**
 * ShiftRegister - Standard 74HC165 PISO Shift Register Reader
 *
 * Reads multiple daisy-chained 74HC165 shift registers using bit-banging.
 * Used for reading encoder and button states on ESP32 peripheral nodes.
 *
 * Typical usage for 16 shift registers (128 bits):
 *   ShiftRegister sr(DATA_PIN, CLOCK_PIN, LATCH_PIN, 16);
 *   sr.begin();
 *   sr.read();
 *   uint8_t byte = sr.getByte(0);
 *   bool bit = sr.getBit(42);
 *
 * Performance: ~10µs per byte on ESP32 @ 240MHz
 */
class ShiftRegister {
public:
    /**
     * Constructor
     * @param dataPin - Serial data input pin (Q7 from last chip)
     * @param clockPin - Clock pin (CLK)
     * @param latchPin - Latch/Load pin (SH/LD)
     * @param numChips - Number of daisy-chained 74HC165 chips
     */
    ShiftRegister(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t numChips);

    /**
     * Initialize GPIO pins
     */
    void begin();

    /**
     * Read all shift registers into internal buffer
     * @return true if successful
     */
    bool read();

    /**
     * Get a specific byte from the buffer
     * @param byteIndex - Byte index (0 to numChips-1)
     * @return Byte value
     */
    uint8_t getByte(uint8_t byteIndex);

    /**
     * Get a specific bit from the buffer
     * @param bitIndex - Bit index (0 to numChips*8-1)
     * @return true if bit is HIGH
     */
    bool getBit(uint16_t bitIndex);

    /**
     * Get raw buffer pointer (read-only)
     * @return Pointer to internal buffer
     */
    const uint8_t* getBuffer() const;

    /**
     * Get number of chips
     */
    uint8_t getNumChips() const { return m_numChips; }

    /**
     * Get last read time (microseconds)
     */
    uint32_t getLastReadTime() const { return m_lastReadTime; }

private:
    uint8_t m_dataPin;
    uint8_t m_clockPin;
    uint8_t m_latchPin;
    uint8_t m_numChips;
    uint8_t* m_buffer;
    uint32_t m_lastReadTime;

    void latch();
    void clockPulse();
};

#endif // SHIFT_REGISTER_H
