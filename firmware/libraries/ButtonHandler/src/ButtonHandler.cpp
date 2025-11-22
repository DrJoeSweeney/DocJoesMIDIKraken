#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(uint16_t numButtons)
    : m_numButtons(numButtons)
    , m_buttons(nullptr)
    , m_activeLow(true)
{
    m_buttons = new ButtonState[m_numButtons];
}

ButtonHandler::~ButtonHandler() {
    delete[] m_buttons;
}

void ButtonHandler::begin(bool activeLow) {
    m_activeLow = activeLow;

    for (uint16_t i = 0; i < m_numButtons; i++) {
        m_buttons[i].history = activeLow ? 0xFF : 0x00;
        m_buttons[i].currentState = false;
        m_buttons[i].lastState = false;
        m_buttons[i].pressedFlag = false;
        m_buttons[i].releasedFlag = false;
        m_buttons[i].pressTime = 0;
    }
}

void ButtonHandler::update(const uint8_t* data, uint16_t offset) {
    uint32_t currentTime = millis();

    for (uint16_t i = 0; i < m_numButtons; i++) {
        uint16_t bitIndex = offset + i;
        uint8_t byteIndex = bitIndex / 8;
        uint8_t bitOffset = bitIndex % 8;

        // Extract bit for this button
        bool rawState = (data[byteIndex] & (1 << bitOffset)) != 0;

        updateButton(&m_buttons[i], rawState);

        // Detect edges
        bool pressed = m_buttons[i].currentState && !m_buttons[i].lastState;
        bool released = !m_buttons[i].currentState && m_buttons[i].lastState;

        if (pressed) {
            m_buttons[i].pressedFlag = true;
            m_buttons[i].pressTime = currentTime;
        }

        if (released) {
            m_buttons[i].releasedFlag = true;
        }

        m_buttons[i].lastState = m_buttons[i].currentState;
    }
}

bool ButtonHandler::isPressed(uint16_t index) {
    if (index >= m_numButtons) {
        return false;
    }

    bool pressed = m_buttons[index].pressedFlag;
    m_buttons[index].pressedFlag = false;  // Clear flag after reading
    return pressed;
}

bool ButtonHandler::isReleased(uint16_t index) {
    if (index >= m_numButtons) {
        return false;
    }

    bool released = m_buttons[index].releasedFlag;
    m_buttons[index].releasedFlag = false;  // Clear flag after reading
    return released;
}

bool ButtonHandler::isHeld(uint16_t index) {
    if (index >= m_numButtons) {
        return false;
    }

    return m_buttons[index].currentState;
}

bool ButtonHandler::isHeldFor(uint16_t index, uint32_t durationMs) {
    if (index >= m_numButtons || !m_buttons[index].currentState) {
        return false;
    }

    uint32_t holdTime = millis() - m_buttons[index].pressTime;
    return holdTime >= durationMs;
}

uint32_t ButtonHandler::getHoldTime(uint16_t index) {
    if (index >= m_numButtons || !m_buttons[index].currentState) {
        return 0;
    }

    return millis() - m_buttons[index].pressTime;
}

void ButtonHandler::clearEvents(uint16_t index) {
    if (index < m_numButtons) {
        m_buttons[index].pressedFlag = false;
        m_buttons[index].releasedFlag = false;
    }
}

void ButtonHandler::updateButton(ButtonState* button, bool rawState) {
    // Shift history and add new sample
    button->history = (button->history << 1) | (rawState ? 1 : 0);

    // Update debounced state
    button->currentState = isDebounced(button->history, m_activeLow);
}

bool ButtonHandler::isDebounced(uint8_t history, bool activeLow) {
    if (activeLow) {
        // For active-low: button pressed when all samples are LOW (0x00)
        return history == 0x00;
    } else {
        // For active-high: button pressed when all samples are HIGH (0xFF)
        return history == 0xFF;
    }
}
