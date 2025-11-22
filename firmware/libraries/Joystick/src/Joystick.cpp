#include "Joystick.h"

Joystick::Joystick(uint8_t xPin, uint8_t yPin, uint8_t buttonPin)
    : m_xPin(xPin)
    , m_yPin(yPin)
    , m_buttonPin(buttonPin)
    , m_centerX(512)
    , m_centerY(512)
    , m_deadZone(5)
    , m_invertX(false)
    , m_invertY(false)
    , m_rawX(512)
    , m_rawY(512)
    , m_filteredX(512)
    , m_filteredY(512)
    , m_buttonState(false)
    , m_lastButtonState(false)
    , m_pressedFlag(false)
    , m_releasedFlag(false)
{
}

void Joystick::begin() {
    pinMode(m_buttonPin, INPUT_PULLUP);
    calibrate();
}

void Joystick::update() {
    // Read and filter analog values
    m_rawX = analogRead(m_xPin);
    m_rawY = analogRead(m_yPin);

    m_filteredX = readFiltered(m_xPin);
    m_filteredY = readFiltered(m_yPin);

    // Update button state
    m_lastButtonState = m_buttonState;
    m_buttonState = digitalRead(m_buttonPin) == LOW;  // Active low

    if (m_buttonState && !m_lastButtonState) {
        m_pressedFlag = true;
    }
    if (!m_buttonState && m_lastButtonState) {
        m_releasedFlag = true;
    }
}

uint16_t Joystick::getPitchBend() const {
    // Convert X-axis (0-1023) to pitch bend (0-16383, center=8192)
    int16_t offset = m_filteredX - m_centerX;
    offset = applyDeadZone(offset, m_deadZone);

    if (m_invertX) {
        offset = -offset;
    }

    int16_t pitchBend = 8192 + map(offset, -512, 512, -8192, 8191);
    return constrain(pitchBend, 0, 16383);
}

uint8_t Joystick::getModulation() const {
    // Convert Y-axis (0-1023) to modulation (0-127)
    int16_t offset = m_filteredY - m_centerY;
    offset = applyDeadZone(offset, m_deadZone);

    if (m_invertY) {
        offset = -offset;
    }

    int16_t mod = 64 + map(offset, -512, 512, -64, 63);
    return constrain(mod, 0, 127);
}

bool Joystick::isButtonPressed() {
    bool pressed = m_pressedFlag;
    m_pressedFlag = false;
    return pressed;
}

bool Joystick::isButtonReleased() {
    bool released = m_releasedFlag;
    m_releasedFlag = false;
    return released;
}

bool Joystick::isButtonHeld() const {
    return m_buttonState;
}

void Joystick::setDeadZone(uint8_t deadZone) {
    m_deadZone = deadZone;
}

void Joystick::calibrate() {
    // Sample center position
    uint32_t sumX = 0, sumY = 0;
    const uint8_t samples = 16;

    for (uint8_t i = 0; i < samples; i++) {
        sumX += analogRead(m_xPin);
        sumY += analogRead(m_yPin);
        delay(10);
    }

    m_centerX = sumX / samples;
    m_centerY = sumY / samples;
}

void Joystick::setInvertX(bool invert) {
    m_invertX = invert;
}

void Joystick::setInvertY(bool invert) {
    m_invertY = invert;
}

uint16_t Joystick::readFiltered(uint8_t pin) {
    // Simple moving average (4 samples)
    static uint16_t history[2][4] = {{512,512,512,512}, {512,512,512,512}};
    static uint8_t index[2] = {0, 0};

    uint8_t pinIndex = (pin == m_xPin) ? 0 : 1;

    uint16_t raw = analogRead(pin);
    history[pinIndex][index[pinIndex]] = raw;
    index[pinIndex] = (index[pinIndex] + 1) % 4;

    uint32_t sum = 0;
    for (uint8_t i = 0; i < 4; i++) {
        sum += history[pinIndex][i];
    }

    return sum / 4;
}

int16_t Joystick::applyDeadZone(int16_t value, uint8_t deadZone) {
    if (abs(value) < deadZone) {
        return 0;
    }

    // Scale value outside dead zone
    if (value > 0) {
        return value - deadZone;
    } else {
        return value + deadZone;
    }
}
