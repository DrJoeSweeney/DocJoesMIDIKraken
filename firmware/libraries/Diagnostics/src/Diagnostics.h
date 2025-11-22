#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <Arduino.h>
#include <Protocol.h>

/**
 * Diagnostics - Performance Monitoring and Statistics
 *
 * Tracks performance metrics, latency, throughput, and system health.
 */
class Diagnostics {
public:
    Diagnostics();

    void begin();
    void update();

    // Record scan cycle time
    void recordScanCycle(uint32_t cycleTimeUs);

    // Record I2C transaction
    void recordI2CTransaction(uint32_t latencyUs, bool success);

    // Record event
    void recordEvent(bool dropped = false);

    // Get metrics
    const DiagnosticMetrics& getMetrics() const;

    // Print diagnostics to serial
    void printDiagnostics();

private:
    DiagnosticMetrics m_metrics;
    uint32_t m_startTime;
};

#endif // DIAGNOSTICS_H
