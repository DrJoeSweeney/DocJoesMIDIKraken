#include "MultiI2CMaster.h"

#if defined(__IMXRT1062__)  // Teensy 4.0/4.1

MultiI2CMaster::MultiI2CMaster()
    : m_queueHead(0)
    , m_queueTail(0)
    , m_lastPollTime(0)
    , m_currentSlave(0)
{
    initSlaves();
}

bool MultiI2CMaster::begin(uint32_t clockSpeed) {
    // Initialize Wire (Bus 0) - ESP32 #1, #2, #3
    Wire.begin();
    Wire.setClock(clockSpeed);

    // Initialize Wire1 (Bus 1) - ESP32 #4, #5, #6
    Wire1.begin();
    Wire1.setClock(clockSpeed);

    // Initialize Wire2 (Bus 2) - ESP32 #7, #8, #9
    Wire2.begin();
    Wire2.setClock(clockSpeed);

    // Health check all slaves
    for (uint8_t i = 0; i < NUM_SLAVES; i++) {
        m_slaves[i].healthy = pingSlave(i);
    }

    return true;
}

void MultiI2CMaster::attachInterrupt(int pin, void (*handler)()) {
    pinMode(pin, INPUT);
    ::attachInterrupt(digitalPinToInterrupt(pin), handler, RISING);
}

void MultiI2CMaster::poll() {
    uint32_t currentTime = micros();

    // Poll next slave in round-robin
    bool success = pollSlave(m_currentSlave);

    if (!success) {
        m_slaves[m_currentSlave].failCount++;
        if (m_slaves[m_currentSlave].failCount > 10) {
            m_slaves[m_currentSlave].healthy = false;
        }
    } else {
        m_slaves[m_currentSlave].failCount = 0;
        m_slaves[m_currentSlave].healthy = true;
    }

    m_slaves[m_currentSlave].lastPollTime = currentTime;

    // Move to next slave
    m_currentSlave = (m_currentSlave + 1) % NUM_SLAVES;

    m_lastPollTime = currentTime;
}

bool MultiI2CMaster::getEvent(EventMessage& event) {
    if (m_queueHead == m_queueTail) {
        return false;  // Queue empty
    }

    event = m_eventQueue[m_queueTail];
    m_queueTail = (m_queueTail + 1) % EVENT_QUEUE_SIZE;
    return true;
}

uint16_t MultiI2CMaster::getQueuedEventCount() const {
    if (m_queueHead >= m_queueTail) {
        return m_queueHead - m_queueTail;
    } else {
        return EVENT_QUEUE_SIZE - (m_queueTail - m_queueHead);
    }
}

bool MultiI2CMaster::sendCommand(uint8_t address, I2CCommand command, const uint8_t* data, uint8_t dataLen) {
    TwoWire* wire = getWireForSlave(address);
    if (!wire) {
        return false;
    }

    wire->beginTransmission(address);
    wire->write((uint8_t)command);

    if (data && dataLen > 0) {
        wire->write(data, dataLen);
    }

    uint8_t result = wire->endTransmission();
    return result == 0;
}

bool MultiI2CMaster::getSlaveStatus(uint8_t address, DiagnosticMetrics& status) {
    // Find slave index
    int8_t slaveIndex = -1;
    for (uint8_t i = 0; i < NUM_SLAVES; i++) {
        if (m_slaves[i].address == address) {
            slaveIndex = i;
            break;
        }
    }

    if (slaveIndex < 0) {
        return false;
    }

    status = m_slaves[slaveIndex].metrics;
    return true;
}

bool MultiI2CMaster::isSlaveHealthy(uint8_t address) {
    for (uint8_t i = 0; i < NUM_SLAVES; i++) {
        if (m_slaves[i].address == address) {
            return m_slaves[i].healthy;
        }
    }
    return false;
}

SystemStatus MultiI2CMaster::getSystemStatus() {
    SystemStatus status;
    memset(&status, 0, sizeof(status));

    for (uint8_t i = 0; i < NUM_SLAVES; i++) {
        status.i2cHealthy[i] = m_slaves[i].healthy;
        status.scanRate[i] = m_slaves[i].metrics.eventsProcessed > 0 ?
            1000000 / m_slaves[i].metrics.avgScanCycleTime : 0;
        status.dropRate[i] = m_slaves[i].metrics.eventsDropped;
    }

    status.uptime = millis() / 1000;
    return status;
}

bool MultiI2CMaster::resetSlave(uint8_t address) {
    return sendCommand(address, CMD_RESET);
}

void MultiI2CMaster::initSlaves() {
    // Bus 0 (Wire): ESP32 #1, #2, #3 (Synth panels)
    m_slaves[0] = {0x08, &Wire, false, 0, 0, {}};
    m_slaves[1] = {0x09, &Wire, false, 0, 0, {}};
    m_slaves[2] = {0x0A, &Wire, false, 0, 0, {}};

    // Bus 1 (Wire1): ESP32 #4, #5, #6 (Synth panels)
    m_slaves[3] = {0x0B, &Wire1, false, 0, 0, {}};
    m_slaves[4] = {0x0C, &Wire1, false, 0, 0, {}};
    m_slaves[5] = {0x0D, &Wire1, false, 0, 0, {}};

    // Bus 2 (Wire2): ESP32 #7, #8, #9 (Synth #7-8, FX #9)
    m_slaves[6] = {0x0E, &Wire2, false, 0, 0, {}};
    m_slaves[7] = {0x0F, &Wire2, false, 0, 0, {}};
    m_slaves[8] = {0x10, &Wire2, false, 0, 0, {}};
}

TwoWire* MultiI2CMaster::getWireForSlave(uint8_t address) {
    for (uint8_t i = 0; i < NUM_SLAVES; i++) {
        if (m_slaves[i].address == address) {
            return m_slaves[i].wire;
        }
    }
    return nullptr;
}

bool MultiI2CMaster::pollSlave(uint8_t slaveIndex) {
    if (slaveIndex >= NUM_SLAVES) {
        return false;
    }

    return readEventsFromSlave(slaveIndex);
}

bool MultiI2CMaster::readEventsFromSlave(uint8_t slaveIndex) {
    SlaveInfo& slave = m_slaves[slaveIndex];

    // Send GET_EVENTS command
    slave.wire->beginTransmission(slave.address);
    slave.wire->write((uint8_t)CMD_GET_EVENTS);
    uint8_t result = slave.wire->endTransmission();

    if (result != 0) {
        return false;
    }

    // Request event data
    uint8_t bytesReceived = slave.wire->requestFrom(slave.address, (uint8_t)49);  // 1 + 6*8 bytes max

    if (bytesReceived == 0) {
        return false;
    }

    // Read event count
    uint8_t numEvents = slave.wire->read();

    // Read events
    for (uint8_t i = 0; i < numEvents && i < 6; i++) {
        if (slave.wire->available() >= sizeof(EventMessage)) {
            EventMessage event;
            slave.wire->readBytes((uint8_t*)&event, sizeof(EventMessage));
            queueEvent(event);
        }
    }

    return true;
}

bool MultiI2CMaster::queueEvent(const EventMessage& event) {
    uint16_t nextHead = (m_queueHead + 1) % EVENT_QUEUE_SIZE;

    if (nextHead == m_queueTail) {
        return false;  // Queue full
    }

    m_eventQueue[m_queueHead] = event;
    m_queueHead = nextHead;
    return true;
}

bool MultiI2CMaster::pingSlave(uint8_t slaveIndex) {
    if (slaveIndex >= NUM_SLAVES) {
        return false;
    }

    SlaveInfo& slave = m_slaves[slaveIndex];

    slave.wire->beginTransmission(slave.address);
    slave.wire->write((uint8_t)CMD_PING);
    uint8_t result = slave.wire->endTransmission();

    return result == 0;
}

#endif // Teensy 4.0
