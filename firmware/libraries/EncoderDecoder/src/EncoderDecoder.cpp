#include "EncoderDecoder.h"

// Gray code state machine lookup table
// [last_state][new_state] = delta
// States: 00, 01, 10, 11 (binary representation of CLK, DT pins)
const int8_t EncoderDecoder::STATE_TABLE[4][4] = {
    // From 00 (0)
    {  0,  1, -1,  0 },  // To: 00(no change), 01(CW), 10(CCW), 11(invalid)
    // From 01 (1)
    { -1,  0,  0,  1 },  // To: 00(CCW), 01(no change), 10(invalid), 11(CW)
    // From 10 (2)
    {  1,  0,  0, -1 },  // To: 00(CW), 01(invalid), 10(no change), 11(CCW)
    // From 11 (3)
    {  0, -1,  1,  0 }   // To: 00(invalid), 01(CCW), 10(CW), 11(no change)
};

EncoderDecoder::EncoderDecoder(uint16_t numEncoders)
    : m_numEncoders(numEncoders)
    , m_encoders(nullptr)
{
    m_encoders = new EncoderState[m_numEncoders];
}

EncoderDecoder::~EncoderDecoder() {
    delete[] m_encoders;
}

void EncoderDecoder::begin() {
    for (uint16_t i = 0; i < m_numEncoders; i++) {
        m_encoders[i].lastState = 0;
        m_encoders[i].delta = 0;
        m_encoders[i].position = 0;
        m_encoders[i].lastChangeTime = 0;
        m_encoders[i].deltaTime = 0;
    }
}

void EncoderDecoder::update(const uint8_t* data) {
    uint32_t currentTime = micros();

    for (uint16_t i = 0; i < m_numEncoders; i++) {
        // Each encoder uses 2 bits: CLK and DT
        uint16_t bitIndex = i * 2;
        uint8_t byteIndex = bitIndex / 8;
        uint8_t bitOffset = bitIndex % 8;

        // Extract 2 bits for this encoder
        uint8_t newBits;
        if (bitOffset <= 6) {
            // Both bits in same byte
            newBits = (data[byteIndex] >> bitOffset) & 0x03;
        } else {
            // Bits span two bytes
            uint8_t lowBit = (data[byteIndex] >> bitOffset) & 0x01;
            uint8_t highBit = (data[byteIndex + 1] & 0x01) << 1;
            newBits = lowBit | highBit;
        }

        decodeEncoder(&m_encoders[i], newBits);

        // Update timing for acceleration
        if (m_encoders[i].delta != 0) {
            m_encoders[i].deltaTime = currentTime - m_encoders[i].lastChangeTime;
            m_encoders[i].lastChangeTime = currentTime;
        }
    }
}

int8_t EncoderDecoder::getDelta(uint16_t index) {
    if (index >= m_numEncoders) {
        return 0;
    }

    int8_t delta = m_encoders[index].delta;
    m_encoders[index].delta = 0;  // Clear delta after reading
    return delta;
}

int8_t EncoderDecoder::getAcceleratedDelta(uint16_t index, uint8_t accelCurve) {
    if (index >= m_numEncoders) {
        return 0;
    }

    int8_t delta = m_encoders[index].delta;
    if (delta == 0) {
        return 0;
    }

    m_encoders[index].delta = 0;  // Clear delta after reading

    // Apply acceleration based on rotation speed
    float accel = calculateAcceleration(m_encoders[index].deltaTime, accelCurve);
    int8_t acceleratedDelta = (int8_t)(delta * accel);

    // Clamp to reasonable range
    if (acceleratedDelta > 127) acceleratedDelta = 127;
    if (acceleratedDelta < -127) acceleratedDelta = -127;

    return acceleratedDelta;
}

int32_t EncoderDecoder::getPosition(uint16_t index) {
    if (index >= m_numEncoders) {
        return 0;
    }
    return m_encoders[index].position;
}

void EncoderDecoder::resetPosition(uint16_t index) {
    if (index < m_numEncoders) {
        m_encoders[index].position = 0;
    }
}

float EncoderDecoder::getSpeed(uint16_t index) {
    if (index >= m_numEncoders || m_encoders[index].deltaTime == 0) {
        return 0.0f;
    }

    // Calculate detents per second
    return 1000000.0f / m_encoders[index].deltaTime;
}

void EncoderDecoder::decodeEncoder(EncoderState* encoder, uint8_t newBits) {
    uint8_t lastState = encoder->lastState;
    int8_t delta = STATE_TABLE[lastState][newBits];

    if (delta != 0) {
        encoder->delta += delta;
        encoder->position += delta;
    }

    encoder->lastState = newBits;
}

float EncoderDecoder::calculateAcceleration(uint32_t deltaTime, uint8_t accelCurve) {
    if (accelCurve == 0 || deltaTime == 0) {
        return 1.0f;  // No acceleration
    }

    // Convert deltaTime to speed (detents per second)
    float speed = 1000000.0f / deltaTime;

    // Acceleration curve:
    // Slow rotation (<2 detents/sec): 1x
    // Fast rotation (>20 detents/sec): up to 8x
    float normalizedSpeed = (speed - 2.0f) / 18.0f;  // 0.0 to 1.0 range
    if (normalizedSpeed < 0.0f) normalizedSpeed = 0.0f;
    if (normalizedSpeed > 1.0f) normalizedSpeed = 1.0f;

    // Apply curve strength (0-255 maps to 0.0-1.0)
    float curveStrength = accelCurve / 255.0f;

    // Calculate acceleration multiplier (1.0 to 8.0)
    float accel = 1.0f + (normalizedSpeed * curveStrength * 7.0f);

    return accel;
}
