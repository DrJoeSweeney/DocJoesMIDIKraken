#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

/**
 * Joystick - 2-Axis Analog Joystick Handler
 *
 * Reads analog joystick for pitch bend (X-axis) and modulation (Y-axis).
 * Includes dead zone, calibration, and smoothing.
 *
 * Typical usage:
 *   Joystick joy(X_PIN, Y_PIN, BUTTON_PIN);
 *   joy.begin();
 *   joy.setDeadZone(5);
 *
 *   // In loop:
 *   joy.update();
 *   uint16_t pitch = joy.getPitchBend();  // 0-16383, center=8192
 *   uint8_t mod = joy.getModulation();     // 0-127
 *   bool pressed = joy.isButtonPressed();
 */
class Joystick {
public:
    Joystick(uint8_t xPin, uint8_t yPin, uint8_t buttonPin);

    void begin();
    void update();

    // Get pitch bend value (14-bit, 0-16383, center=8192)
    uint16_t getPitchBend() const;

    // Get modulation value (7-bit, 0-127)
    uint8_t getModulation() const;

    // Button
    bool isButtonPressed();
    bool isButtonReleased();
    bool isButtonHeld() const;

    // Configuration
    void setDeadZone(uint8_t deadZone);  // 0-127
    void calibrate();  // Auto-calibrate center
    void setInvertX(bool invert);
    void setInvertY(bool invert);

private:
    uint8_t m_xPin, m_yPin, m_buttonPin;
    uint16_t m_centerX, m_centerY;
    uint8_t m_deadZone;
    bool m_invertX, m_invertY;

    uint16_t m_rawX, m_rawY;
    uint16_t m_filteredX, m_filteredY;

    bool m_buttonState, m_lastButtonState;
    bool m_pressedFlag, m_releasedFlag;

    uint16_t readFiltered(uint8_t pin);
    int16_t applyDeadZone(int16_t value, uint8_t deadZone);
};

#endif // JOYSTICK_H
