#ifndef MULTI_I2C_MASTER_H
#define MULTI_I2C_MASTER_H

#include <Arduino.h>
#include <Protocol.h>

#if defined(__IMXRT1062__)  // Teensy 4.0/4.1
#include <Wire.h>

/**
 * MultiI2CMaster - Teensy 4.0 Multi-Bus I2C Master
 *
 * Manages 3 parallel I2C buses for distributed ESP32 communication.
 * Uses interrupt-driven polling for sub-500µs latency.
 *
 * Bus Assignment (32-Encoder Panel Design):
 * - Bus 0 (Wire): ESP32 #1 (0x08), #2 (0x09), #3 (0x0A)
 * - Bus 1 (Wire1): ESP32 #4 (0x0B), #5 (0x0C), #6 (0x0D)
 * - Bus 2 (Wire2): ESP32 #7 (0x0E), #8 (0x0F), #9 (0x10)
 *
 * Features:
 * - 3 parallel I2C buses (1MHz Fast Mode+)
 * - Interrupt-driven event polling
 * - Event batching (up to 6 events per slave)
 * - Health monitoring
 * - Automatic retry on failure
 *
 * Typical usage:
 *   MultiI2CMaster master;
 *   master.begin();
 *   master.attachInterrupt(eventPin, []() { master.handleInterrupt(); });
 *
 *   // In loop:
 *   master.poll();
 *   EventMessage event;
 *   if (master.getEvent(event)) { ... }
 */
class MultiI2CMaster {
public:
    /**
     * Constructor
     */
    MultiI2CMaster();

    /**
     * Initialize all I2C buses
     * @param clockSpeed - Clock speed for all buses (default 1MHz)
     * @return true if all buses initialized successfully
     */
    bool begin(uint32_t clockSpeed = 1000000);

    /**
     * Attach interrupt handler for event pins
     * @param pin - Event interrupt pin
     * @param handler - Interrupt handler function
     */
    void attachInterrupt(int pin, void (*handler)());

    /**
     * Poll all slaves for events
     * Call this regularly in loop() or from interrupt
     */
    void poll();

    /**
     * Get next event from global event queue
     * @param event - Output parameter for event
     * @return true if event retrieved, false if queue empty
     */
    bool getEvent(EventMessage& event);

    /**
     * Get number of queued events
     */
    uint16_t getQueuedEventCount() const;

    /**
     * Send command to specific ESP32
     * @param address - Slave address (0x08 to 0x10)
     * @param command - Command code
     * @param data - Optional data bytes
     * @param dataLen - Length of data
     * @return true if successful
     */
    bool sendCommand(uint8_t address, I2CCommand command, const uint8_t* data = nullptr, uint8_t dataLen = 0);

    /**
     * Get status from specific ESP32
     * @param address - Slave address
     * @param status - Output parameter for status
     * @return true if successful
     */
    bool getSlaveStatus(uint8_t address, DiagnosticMetrics& status);

    /**
     * Check if slave is healthy
     * @param address - Slave address
     * @return true if slave responding
     */
    bool isSlaveHealthy(uint8_t address);

    /**
     * Get system status for all slaves
     */
    SystemStatus getSystemStatus();

    /**
     * Reset specific slave
     * @param address - Slave address
     * @return true if successful
     */
    bool resetSlave(uint8_t address);

private:
    struct SlaveInfo {
        uint8_t address;
        TwoWire* wire;
        bool healthy;
        uint32_t lastPollTime;
        uint32_t failCount;
        DiagnosticMetrics metrics;
    };

    static const uint8_t NUM_SLAVES = 9;  // 8 synth panels + 1 FX panel (snapshot via separate interface)
    static const uint16_t EVENT_QUEUE_SIZE = 256;

    SlaveInfo m_slaves[NUM_SLAVES];
    EventMessage m_eventQueue[EVENT_QUEUE_SIZE];
    volatile uint16_t m_queueHead;
    volatile uint16_t m_queueTail;

    uint32_t m_lastPollTime;
    uint8_t m_currentSlave;

    /**
     * Initialize slave info
     */
    void initSlaves();

    /**
     * Get Wire interface for slave address
     */
    TwoWire* getWireForSlave(uint8_t address);

    /**
     * Poll specific slave
     */
    bool pollSlave(uint8_t slaveIndex);

    /**
     * Read events from slave
     */
    bool readEventsFromSlave(uint8_t slaveIndex);

    /**
     * Queue event to global queue
     */
    bool queueEvent(const EventMessage& event);

    /**
     * Check slave health with ping
     */
    bool pingSlave(uint8_t slaveIndex);
};

#endif // Teensy 4.0
#endif // MULTI_I2C_MASTER_H
