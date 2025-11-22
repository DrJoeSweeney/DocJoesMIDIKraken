#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

/**
 * ButtonHandler - Debounced Button Input Handler
 *
 * Handles button debouncing using 8-sample history method.
 * Detects press, release, and hold events reliably.
 *
 * Features:
 * - 8-sample debouncing (stable for 8 consecutive reads)
 * - Press and release detection
 * - Hold detection with configurable duration
 * - Active-low or active-high configuration
 *
 * Typical usage:
 *   ButtonHandler buttons(NUM_BUTTONS);
 *   buttons.begin(true);  // Active low
 *   buttons.update(shiftRegisterData);
 *   if (buttons.isPressed(buttonIndex)) { ... }
 *   if (buttons.isReleased(buttonIndex)) { ... }
 */
class ButtonHandler {
public:
    /**
     * Constructor
     * @param numButtons - Number of buttons to track
     */
    ButtonHandler(uint16_t numButtons);
    ~ButtonHandler();

    /**
     * Initialize button handler
     * @param activeLow - true if buttons are active-low (default for most switches)
     */
    void begin(bool activeLow = true);

    /**
     * Update button states from shift register data
     * @param data - Pointer to shift register buffer
     * @param offset - Bit offset to start reading buttons from
     */
    void update(const uint8_t* data, uint16_t offset = 0);

    /**
     * Check if button was just pressed (transition from released to pressed)
     * @param index - Button index
     * @return true if button was pressed this update
     */
    bool isPressed(uint16_t index);

    /**
     * Check if button was just released (transition from pressed to released)
     * @param index - Button index
     * @return true if button was released this update
     */
    bool isReleased(uint16_t index);

    /**
     * Check if button is currently held down
     * @param index - Button index
     * @return true if button is currently pressed
     */
    bool isHeld(uint16_t index);

    /**
     * Check if button has been held for duration
     * @param index - Button index
     * @param durationMs - Hold duration in milliseconds
     * @return true if button held for at least durationMs
     */
    bool isHeldFor(uint16_t index, uint32_t durationMs);

    /**
     * Get time button has been held (milliseconds)
     * @param index - Button index
     * @return Hold time in ms, or 0 if not held
     */
    uint32_t getHoldTime(uint16_t index);

    /**
     * Clear press/release flags for a button
     * @param index - Button index
     */
    void clearEvents(uint16_t index);

private:
    struct ButtonState {
        uint8_t history;        // 8-bit debounce history
        bool currentState;      // Current debounced state (true = pressed)
        bool lastState;         // Previous state (for edge detection)
        bool pressedFlag;       // Set on press, cleared after read
        bool releasedFlag;      // Set on release, cleared after read
        uint32_t pressTime;     // Time of last press (ms)
    };

    uint16_t m_numButtons;
    ButtonState* m_buttons;
    bool m_activeLow;

    /**
     * Update debounce history for one button
     * @param button - Pointer to button state
     * @param rawState - Raw button state from shift register
     */
    void updateButton(ButtonState* button, bool rawState);

    /**
     * Check if button is debounced stable
     * @param history - 8-bit history
     * @param activeLow - Active low configuration
     * @return true if button is pressed (debounced)
     */
    bool isDebounced(uint8_t history, bool activeLow);
};

#endif // BUTTON_HANDLER_H
