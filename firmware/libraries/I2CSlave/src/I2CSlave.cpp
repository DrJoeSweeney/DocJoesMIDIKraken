#include "I2CSlave.h"

#ifdef ESP32

I2CSlave* I2CSlave::s_instance = nullptr;

I2CSlave::I2CSlave(uint8_t address, TwoWire& wire, int eventPin)
    : m_address(address)
    , m_wire(wire)
    , m_eventPin(eventPin)
    , m_queueHead(0)
    , m_queueTail(0)
{
    s_instance = this;
    memset(&m_metrics, 0, sizeof(m_metrics));
}

bool I2CSlave::begin(int sdaPin, int sclPin, uint32_t clockSpeed) {
    // Initialize Wire as slave
    if (!m_wire.begin(m_address, sdaPin, sclPin, clockSpeed)) {
        return false;
    }

    // Set up callbacks
    m_wire.onRequest(onRequestStatic);
    m_wire.onReceive(onReceiveStatic);

    // Configure event pin if provided
    if (m_eventPin >= 0) {
        pinMode(m_eventPin, OUTPUT);
        clearEventSignal();
    }

    return true;
}

void I2CSlave::update() {
    // Signal if events are queued
    if (m_queueHead != m_queueTail && m_eventPin >= 0) {
        signalEvent();
    } else if (m_eventPin >= 0) {
        clearEventSignal();
    }
}

bool I2CSlave::queueEvent(const EventMessage& event) {
    uint16_t nextHead = (m_queueHead + 1) % MAX_QUEUED_EVENTS;

    if (nextHead == m_queueTail) {
        return false;  // Queue full
    }

    m_eventQueue[m_queueHead] = event;
    m_queueHead = nextHead;

    return true;
}

uint16_t I2CSlave::getQueuedEventCount() const {
    if (m_queueHead >= m_queueTail) {
        return m_queueHead - m_queueTail;
    } else {
        return MAX_QUEUED_EVENTS - (m_queueTail - m_queueHead);
    }
}

void I2CSlave::setStatus(const DiagnosticMetrics& metrics) {
    m_metrics = metrics;
}

const DiagnosticMetrics& I2CSlave::getMetrics() const {
    return m_metrics;
}

void I2CSlave::reset() {
    m_queueHead = 0;
    m_queueTail = 0;
    memset(&m_metrics, 0, sizeof(m_metrics));
    clearEventSignal();
}

void I2CSlave::onRequestStatic() {
    if (s_instance) {
        s_instance->onRequest();
    }
}

void I2CSlave::onReceiveStatic(int numBytes) {
    if (s_instance) {
        s_instance->onReceive(numBytes);
    }
}

void I2CSlave::onRequest() {
    // Master is requesting data
    // Send queued events (up to 6 events, 8 bytes each = 48 bytes)
    const uint16_t MAX_EVENTS_PER_TRANSACTION = 6;
    uint8_t numEvents = min(getQueuedEventCount(), MAX_EVENTS_PER_TRANSACTION);

    // Send event count first
    m_wire.write(numEvents);

    // Send events
    for (uint8_t i = 0; i < numEvents; i++) {
        const EventMessage& event = m_eventQueue[m_queueTail];
        m_wire.write((uint8_t*)&event, sizeof(EventMessage));

        m_queueTail = (m_queueTail + 1) % MAX_QUEUED_EVENTS;
    }
}

void I2CSlave::onReceive(int numBytes) {
    if (numBytes == 0) {
        return;
    }

    // Read command byte
    uint8_t command = m_wire.read();

    switch (command) {
        case CMD_GET_EVENTS:
            handleGetEventsCommand();
            break;

        case CMD_GET_STATUS:
            handleGetStatusCommand();
            break;

        case CMD_PING:
            handlePingCommand();
            break;

        case CMD_RESET:
            handleResetCommand();
            break;

        default:
            // Unknown command, drain remaining bytes
            while (m_wire.available()) {
                m_wire.read();
            }
            break;
    }
}

void I2CSlave::handleGetEventsCommand() {
    // Master will call onRequest() next
}

void I2CSlave::handleGetStatusCommand() {
    // Master will call onRequest() to get status data
    // Status is sent in next request cycle
}

void I2CSlave::handlePingCommand() {
    // Respond with ACK (handled by I2C protocol)
}

void I2CSlave::handleResetCommand() {
    reset();
}

void I2CSlave::signalEvent() {
    if (m_eventPin >= 0) {
        digitalWrite(m_eventPin, HIGH);
    }
}

void I2CSlave::clearEventSignal() {
    if (m_eventPin >= 0) {
        digitalWrite(m_eventPin, LOW);
    }
}

#endif // ESP32
