#ifndef ENCODER_DECODER_H
#define ENCODER_DECODER_H

#include <Arduino.h>

/**
 * EncoderDecoder - Quadrature Rotary Encoder Decoder
 *
 * Decodes quadrature encoder signals using Gray code state machine.
 * Handles acceleration curves for faster movement detection.
 *
 * Features:
 * - Gray code state machine (4 valid states)
 * - Acceleration detection based on timing
 * - Debouncing through state validation
 * - Configurable acceleration curves
 *
 * Typical usage:
 *   EncoderDecoder dec(NUM_ENCODERS);
 *   dec.begin();
 *   dec.update(shiftRegisterData);
 *   int8_t delta = dec.getDelta(encoderIndex);
 *   int8_t accelDelta = dec.getAcceleratedDelta(encoderIndex, accelCurve);
 */
class EncoderDecoder {
public:
    /**
     * Constructor
     * @param numEncoders - Number of encoders to track
     */
    EncoderDecoder(uint16_t numEncoders);
    ~EncoderDecoder();

    /**
     * Initialize encoder state
     */
    void begin();

    /**
     * Update encoder states from shift register data
     * @param data - Pointer to shift register buffer
     *               Data format: 2 bits per encoder (CLK, DT)
     */
    void update(const uint8_t* data);

    /**
     * Get raw delta for an encoder since last update
     * @param index - Encoder index (0 to numEncoders-1)
     * @return Delta value (-N to +N, typically -1, 0, +1)
     */
    int8_t getDelta(uint16_t index);

    /**
     * Get accelerated delta based on rotation speed
     * @param index - Encoder index
     * @param accelCurve - Acceleration multiplier (0-255)
     *                     0 = no acceleration, 255 = max acceleration
     * @return Accelerated delta value
     */
    int8_t getAcceleratedDelta(uint16_t index, uint8_t accelCurve);

    /**
     * Get current position (accumulated delta)
     * @param index - Encoder index
     * @return Position value
     */
    int32_t getPosition(uint16_t index);

    /**
     * Reset encoder position to zero
     * @param index - Encoder index
     */
    void resetPosition(uint16_t index);

    /**
     * Get rotation speed (detents per second)
     * @param index - Encoder index
     * @return Speed in detents/second
     */
    float getSpeed(uint16_t index);

private:
    struct EncoderState {
        uint8_t lastState;      // Last Gray code state (2 bits)
        int8_t delta;           // Current delta since last read
        int32_t position;       // Accumulated position
        uint32_t lastChangeTime;// Last state change (µs)
        uint32_t deltaTime;     // Time between changes (µs)
    };

    uint16_t m_numEncoders;
    EncoderState* m_encoders;

    // Gray code state machine lookup table
    // Returns -1 (CCW), 0 (invalid/no change), +1 (CW)
    static const int8_t STATE_TABLE[4][4];

    /**
     * Decode one encoder's new state
     * @param encoder - Pointer to encoder state
     * @param newBits - New 2-bit Gray code state
     */
    void decodeEncoder(EncoderState* encoder, uint8_t newBits);

    /**
     * Calculate acceleration multiplier based on speed
     * @param deltaTime - Time between detents (µs)
     * @param accelCurve - Acceleration curve (0-255)
     * @return Acceleration multiplier (1.0 to 8.0)
     */
    float calculateAcceleration(uint32_t deltaTime, uint8_t accelCurve);
};

#endif // ENCODER_DECODER_H
