#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#include <Arduino.h>
#include <Protocol.h>

#ifdef ESP32
#include <Wire.h>

/**
 * I2CSlave - ESP32 I2C Slave Interface with Interrupt Handling
 *
 * Handles ESP32 as I2C slave device responding to Teensy master requests.
 * Uses interrupt-driven communication for <500µs latency.
 *
 * Features:
 * - Interrupt-driven (responds to master immediately)
 * - Event queue for batching up to 6 events per transaction
 * - Command-response protocol
 * - Status reporting
 *
 * Typical usage:
 *   I2CSlave slave(0x08, Wire, EVENT_PIN);
 *   slave.begin();
 *   slave.queueEvent(event);
 *
 *   // In loop:
 *   slave.update();
 */
class I2CSlave {
public:
    /**
     * Constructor
     * @param address - I2C slave address (0x08 to 0x0E for ESP32 #1-7)
     * @param wire - Wire interface reference
     * @param eventPin - GPIO pin to signal events to master (optional)
     */
    I2CSlave(uint8_t address, TwoWire& wire, int eventPin = -1);

    /**
     * Initialize I2C slave
     * @param sdaPin - SDA pin
     * @param sclPin - SCL pin
     * @param clockSpeed - I2C clock speed (default 1MHz Fast Mode+)
     * @return true if successful
     */
    bool begin(int sdaPin, int sclPin, uint32_t clockSpeed = 1000000);

    /**
     * Update I2C slave (call in loop)
     */
    void update();

    /**
     * Queue event to send to master
     * @param event - EventMessage to queue
     * @return true if queued, false if queue full
     */
    bool queueEvent(const EventMessage& event);

    /**
     * Get number of queued events
     */
    uint16_t getQueuedEventCount() const;

    /**
     * Set status information
     */
    void setStatus(const DiagnosticMetrics& metrics);

    /**
     * Get diagnostic metrics
     */
    const DiagnosticMetrics& getMetrics() const;

    /**
     * Reset slave (clear queues, reset state)
     */
    void reset();

private:
    uint8_t m_address;
    TwoWire& m_wire;
    int m_eventPin;

    // Event queue (up to 6 events per transaction)
    static const uint16_t MAX_QUEUED_EVENTS = 128;
    EventMessage m_eventQueue[MAX_QUEUED_EVENTS];
    volatile uint16_t m_queueHead;
    volatile uint16_t m_queueTail;

    // Status
    DiagnosticMetrics m_metrics;

    // Callbacks (static for ISR compatibility)
    static void onRequestStatic();
    static void onReceiveStatic(int numBytes);
    static I2CSlave* s_instance;

    void onRequest();
    void onReceive(int numBytes);

    void handleGetEventsCommand();
    void handleGetStatusCommand();
    void handlePingCommand();
    void handleResetCommand();

    void signalEvent();
    void clearEventSignal();
};

#endif // ESP32
#endif // I2C_SLAVE_H
