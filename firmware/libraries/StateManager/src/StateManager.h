#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>
#include <Protocol.h>

/**
 * StateManager - Centralized State Management for All 619 Controls
 *
 * Manages real-time state for all controls including current values,
 * configurations, and runtime flags. Provides fast lookup and update operations.
 *
 * Features:
 * - 619 control states (O(1) access)
 * - Configuration management
 * - Dirty flag tracking
 * - Value change detection
 * - Bank A/B switching
 *
 * Typical usage:
 *   StateManager state;
 *   state.begin();
 *   state.setValue(globalID, 64);
 *   uint8_t value = state.getValue(globalID);
 *   if (state.isDirty(globalID)) { sendMIDI(...); state.clearDirty(globalID); }
 */
class StateManager {
public:
    StateManager();
    ~StateManager();

    /**
     * Initialize state manager
     */
    void begin();

    /**
     * Set control value
     * @param globalID - Control ID (0-618)
     * @param value - MIDI value (0-127)
     * @return true if value changed
     */
    bool setValue(uint16_t globalID, uint8_t value);

    /**
     * Get control value
     * @param globalID - Control ID
     * @return Current value
     */
    uint8_t getValue(uint16_t globalID) const;

    /**
     * Get target value (for morphing)
     */
    uint8_t getTargetValue(uint16_t globalID) const;

    /**
     * Set target value (for morphing)
     */
    void setTargetValue(uint16_t globalID, uint8_t value);

    /**
     * Check if control value changed (dirty flag)
     */
    bool isDirty(uint16_t globalID) const;

    /**
     * Clear dirty flag
     */
    void clearDirty(uint16_t globalID);

    /**
     * Mark control as dirty
     */
    void markDirty(uint16_t globalID);

    /**
     * Get control configuration
     */
    ControlConfig* getConfig(uint16_t globalID);
    const ControlConfig* getConfig(uint16_t globalID) const;

    /**
     * Set control configuration
     */
    void setConfig(uint16_t globalID, const ControlConfig& config);

    /**
     * Get control state
     */
    ControlState* getState(uint16_t globalID);
    const ControlState* getState(uint16_t globalID) const;

    /**
     * Switch Bank A/B
     * @param useB - true for Bank B, false for Bank A
     */
    void setBank(bool useB);

    /**
     * Get current bank
     * @return true if Bank B active
     */
    bool isBankB() const { return m_currentBank; }

    /**
     * Load all states from snapshot values
     */
    void loadSnapshot(const Snapshot& snapshot);

    /**
     * Save all values to snapshot
     */
    void saveSnapshot(Snapshot& snapshot) const;

    /**
     * Get total number of controls
     */
    uint16_t getTotalControls() const { return TOTAL_CONTROLS; }

private:
    ControlConfig* m_configs;
    ControlState* m_states;
    bool m_currentBank;  // false = A, true = B
};

#endif // STATE_MANAGER_H
