#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <Arduino.h>
#include <Protocol.h>
#include <Wire.h>

/**
 * I2CMaster - Simple Single-Bus I2C Master
 *
 * Simplified I2C master for single-bus communication with multiple slaves.
 * Useful for testing, prototyping, or simpler controller configurations.
 *
 * Features:
 * - Single I2C bus communication
 * - Sequential slave polling
 * - Event queuing
 * - Health monitoring
 *
 * Typical usage:
 *   I2CMaster master(Wire);
 *   master.addSlave(0x08);
 *   master.addSlave(0x09);
 *   master.begin();
 *
 *   // In loop:
 *   master.poll();
 *   EventMessage event;
 *   if (master.getEvent(event)) { ... }
 */
class I2CMaster {
public:
    /**
     * Constructor
     * @param wire - Wire interface reference
     */
    I2CMaster(TwoWire& wire);

    /**
     * Add slave to poll list
     * @param address - Slave I2C address
     * @return true if added successfully
     */
    bool addSlave(uint8_t address);

    /**
     * Initialize I2C master
     * @param clockSpeed - I2C clock speed (default 400kHz)
     * @return true if successful
     */
    bool begin(uint32_t clockSpeed = 400000);

    /**
     * Poll all slaves for events
     */
    void poll();

    /**
     * Get next event from queue
     * @param event - Output parameter for event
     * @return true if event retrieved
     */
    bool getEvent(EventMessage& event);

    /**
     * Get number of queued events
     */
    uint16_t getQueuedEventCount() const;

    /**
     * Send command to slave
     * @param address - Slave address
     * @param command - Command code
     * @return true if successful
     */
    bool sendCommand(uint8_t address, I2CCommand command);

    /**
     * Check if slave is healthy
     * @param address - Slave address
     * @return true if responding
     */
    bool isSlaveHealthy(uint8_t address);

private:
    TwoWire& m_wire;

    static const uint8_t MAX_SLAVES = 8;
    static const uint16_t EVENT_QUEUE_SIZE = 128;

    uint8_t m_slaveAddresses[MAX_SLAVES];
    uint8_t m_numSlaves;

    EventMessage m_eventQueue[EVENT_QUEUE_SIZE];
    uint16_t m_queueHead;
    uint16_t m_queueTail;

    bool readEventsFromSlave(uint8_t address);
    bool queueEvent(const EventMessage& event);
};

#endif // I2C_MASTER_H
