#include "StateManager.h"

StateManager::StateManager()
    : m_configs(nullptr)
    , m_states(nullptr)
    , m_currentBank(false)
{
    m_configs = new ControlConfig[TOTAL_CONTROLS];
    m_states = new ControlState[TOTAL_CONTROLS];
}

StateManager::~StateManager() {
    delete[] m_configs;
    delete[] m_states;
}

void StateManager::begin() {
    // Initialize all configs with defaults
    for (uint16_t i = 0; i < TOTAL_CONTROLS; i++) {
        m_configs[i].globalID = i;
        m_configs[i].controlType = CONTROL_ENCODER;
        m_configs[i].flags = CONTROL_FLAG_ENABLED;
        m_configs[i].snapshotFlags = 0;
        m_configs[i].ccNumber = i % 128;
        m_configs[i].midiChannel = 0;
        m_configs[i].virtualDevice = 0;
        m_configs[i].resolution = 0;  // 7-bit
        m_configs[i].minValue = 0;
        m_configs[i].maxValue = 127;
        m_configs[i].defaultValue = 64;
        m_configs[i].currentBank = 0;
        m_configs[i].encoderMode = ENCODER_ABSOLUTE;
        m_configs[i].acceleration = 128;
        m_configs[i].threshold = 1;
        m_configs[i].deadZone = 0;
        m_configs[i].buttonAction = BUTTON_CC_TOGGLE;
        m_configs[i].buttonValue = 127;
        m_configs[i].buttonToggleState = 0;
        m_configs[i].ccNumberB = i % 128;
        m_configs[i].midiChannelB = 0;
        m_configs[i].virtualDeviceB = 0;
        m_configs[i].resolutionB = 0;
        m_configs[i].panelID = 0;
        m_configs[i].groupID = 0;
        m_configs[i].groupFlags = 0;
        snprintf(m_configs[i].label, sizeof(m_configs[i].label), "Control %d", i);
        m_configs[i].eventCount = 0;
        m_configs[i].lastEventTime = 0;

        m_states[i].value = 64;
        m_states[i].lastSentValue = 64;
        m_states[i].targetValue = 64;
        m_states[i].stateFlags = 0;
    }
}

bool StateManager::setValue(uint16_t globalID, uint8_t value) {
    if (globalID >= TOTAL_CONTROLS) {
        return false;
    }

    if (m_states[globalID].value != value) {
        m_states[globalID].value = value;
        m_states[globalID].stateFlags |= STATE_FLAG_DIRTY;
        return true;
    }

    return false;
}

uint8_t StateManager::getValue(uint16_t globalID) const {
    if (globalID >= TOTAL_CONTROLS) {
        return 0;
    }
    return m_states[globalID].value;
}

uint8_t StateManager::getTargetValue(uint16_t globalID) const {
    if (globalID >= TOTAL_CONTROLS) {
        return 0;
    }
    return m_states[globalID].targetValue;
}

void StateManager::setTargetValue(uint16_t globalID, uint8_t value) {
    if (globalID < TOTAL_CONTROLS) {
        m_states[globalID].targetValue = value;
        m_states[globalID].stateFlags |= STATE_FLAG_TRANSITIONING;
    }
}

bool StateManager::isDirty(uint16_t globalID) const {
    if (globalID >= TOTAL_CONTROLS) {
        return false;
    }
    return (m_states[globalID].stateFlags & STATE_FLAG_DIRTY) != 0;
}

void StateManager::clearDirty(uint16_t globalID) {
    if (globalID < TOTAL_CONTROLS) {
        m_states[globalID].stateFlags &= ~STATE_FLAG_DIRTY;
        m_states[globalID].lastSentValue = m_states[globalID].value;
    }
}

void StateManager::markDirty(uint16_t globalID) {
    if (globalID < TOTAL_CONTROLS) {
        m_states[globalID].stateFlags |= STATE_FLAG_DIRTY;
    }
}

ControlConfig* StateManager::getConfig(uint16_t globalID) {
    if (globalID >= TOTAL_CONTROLS) {
        return nullptr;
    }
    return &m_configs[globalID];
}

const ControlConfig* StateManager::getConfig(uint16_t globalID) const {
    if (globalID >= TOTAL_CONTROLS) {
        return nullptr;
    }
    return &m_configs[globalID];
}

void StateManager::setConfig(uint16_t globalID, const ControlConfig& config) {
    if (globalID < TOTAL_CONTROLS) {
        m_configs[globalID] = config;
    }
}

ControlState* StateManager::getState(uint16_t globalID) {
    if (globalID >= TOTAL_CONTROLS) {
        return nullptr;
    }
    return &m_states[globalID];
}

const ControlState* StateManager::getState(uint16_t globalID) const {
    if (globalID >= TOTAL_CONTROLS) {
        return nullptr;
    }
    return &m_states[globalID];
}

void StateManager::setBank(bool useB) {
    m_currentBank = useB;

    // Update all configs to use new bank
    for (uint16_t i = 0; i < TOTAL_CONTROLS; i++) {
        m_configs[i].currentBank = useB ? 1 : 0;
    }
}

void StateManager::loadSnapshot(const Snapshot& snapshot) {
    for (uint16_t i = 0; i < TOTAL_CONTROLS; i++) {
        // Check zero mask
        uint16_t maskIndex = i / 16;
        uint16_t maskBit = i % 16;
        bool zeroed = (snapshot.zeroMask[maskIndex] & (1 << maskBit)) != 0;

        if (!zeroed) {
            setValue(i, snapshot.values[i]);
        }
    }
}

void StateManager::saveSnapshot(Snapshot& snapshot) const {
    for (uint16_t i = 0; i < TOTAL_CONTROLS; i++) {
        snapshot.values[i] = m_states[i].value;
    }
}
