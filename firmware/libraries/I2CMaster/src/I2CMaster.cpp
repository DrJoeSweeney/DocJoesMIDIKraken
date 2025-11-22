#include "I2CMaster.h"

I2CMaster::I2CMaster(TwoWire& wire)
    : m_wire(wire)
    , m_numSlaves(0)
    , m_queueHead(0)
    , m_queueTail(0)
{
}

bool I2CMaster::addSlave(uint8_t address) {
    if (m_numSlaves >= MAX_SLAVES) {
        return false;
    }

    m_slaveAddresses[m_numSlaves++] = address;
    return true;
}

bool I2CMaster::begin(uint32_t clockSpeed) {
    m_wire.begin();
    m_wire.setClock(clockSpeed);
    return true;
}

void I2CMaster::poll() {
    for (uint8_t i = 0; i < m_numSlaves; i++) {
        readEventsFromSlave(m_slaveAddresses[i]);
    }
}

bool I2CMaster::getEvent(EventMessage& event) {
    if (m_queueHead == m_queueTail) {
        return false;
    }

    event = m_eventQueue[m_queueTail];
    m_queueTail = (m_queueTail + 1) % EVENT_QUEUE_SIZE;
    return true;
}

uint16_t I2CMaster::getQueuedEventCount() const {
    if (m_queueHead >= m_queueTail) {
        return m_queueHead - m_queueTail;
    } else {
        return EVENT_QUEUE_SIZE - (m_queueTail - m_queueHead);
    }
}

bool I2CMaster::sendCommand(uint8_t address, I2CCommand command) {
    m_wire.beginTransmission(address);
    m_wire.write((uint8_t)command);
    uint8_t result = m_wire.endTransmission();
    return result == 0;
}

bool I2CMaster::isSlaveHealthy(uint8_t address) {
    m_wire.beginTransmission(address);
    m_wire.write((uint8_t)CMD_PING);
    uint8_t result = m_wire.endTransmission();
    return result == 0;
}

bool I2CMaster::readEventsFromSlave(uint8_t address) {
    // Send GET_EVENTS command
    m_wire.beginTransmission(address);
    m_wire.write((uint8_t)CMD_GET_EVENTS);
    uint8_t result = m_wire.endTransmission();

    if (result != 0) {
        return false;
    }

    // Request event data
    uint8_t bytesReceived = m_wire.requestFrom(address, (uint8_t)49);

    if (bytesReceived == 0) {
        return false;
    }

    // Read event count
    uint8_t numEvents = m_wire.read();

    // Read events
    for (uint8_t i = 0; i < numEvents && i < 6; i++) {
        if (m_wire.available() >= sizeof(EventMessage)) {
            EventMessage event;
            m_wire.readBytes((uint8_t*)&event, sizeof(EventMessage));
            queueEvent(event);
        }
    }

    return true;
}

bool I2CMaster::queueEvent(const EventMessage& event) {
    uint16_t nextHead = (m_queueHead + 1) % EVENT_QUEUE_SIZE;

    if (nextHead == m_queueTail) {
        return false;
    }

    m_eventQueue[m_queueHead] = event;
    m_queueHead = nextHead;
    return true;
}
