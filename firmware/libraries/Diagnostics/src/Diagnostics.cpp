#include "Diagnostics.h"

Diagnostics::Diagnostics() : m_startTime(0) {
    memset(&m_metrics, 0, sizeof(m_metrics));
}

void Diagnostics::begin() {
    m_startTime = millis();
    memset(&m_metrics, 0, sizeof(m_metrics));
}

void Diagnostics::update() {
    // Calculate drop rate
    if (m_metrics.eventsProcessed > 0) {
        m_metrics.dropRate = (m_metrics.eventsDropped * 100.0f) /
            (m_metrics.eventsProcessed + m_metrics.eventsDropped);
    }
}

void Diagnostics::recordScanCycle(uint32_t cycleTimeUs) {
    m_metrics.scanCycleTime = cycleTimeUs;

    // Update average (simple moving average)
    if (m_metrics.avgScanCycleTime == 0) {
        m_metrics.avgScanCycleTime = cycleTimeUs;
    } else {
        m_metrics.avgScanCycleTime = (m_metrics.avgScanCycleTime * 15 + cycleTimeUs) / 16;
    }

    // Update max
    if (cycleTimeUs > m_metrics.maxScanCycleTime) {
        m_metrics.maxScanCycleTime = cycleTimeUs;
    }
}

void Diagnostics::recordI2CTransaction(uint32_t latencyUs, bool success) {
    m_metrics.i2cLatency = latencyUs;
}

void Diagnostics::recordEvent(bool dropped) {
    if (dropped) {
        m_metrics.eventsDropped++;
    } else {
        m_metrics.eventsProcessed++;
    }
}

const DiagnosticMetrics& Diagnostics::getMetrics() const {
    return m_metrics;
}

void Diagnostics::printDiagnostics() {
    Serial.println("=== Diagnostics ===");
    Serial.print("Scan cycle: ");
    Serial.print(m_metrics.scanCycleTime);
    Serial.print(" us (avg: ");
    Serial.print(m_metrics.avgScanCycleTime);
    Serial.print(" us, max: ");
    Serial.print(m_metrics.maxScanCycleTime);
    Serial.println(" us)");

    Serial.print("I2C latency: ");
    Serial.print(m_metrics.i2cLatency);
    Serial.println(" us");

    Serial.print("Events: ");
    Serial.print(m_metrics.eventsProcessed);
    Serial.print(" processed, ");
    Serial.print(m_metrics.eventsDropped);
    Serial.print(" dropped (");
    Serial.print(m_metrics.dropRate, 2);
    Serial.println("%)");

    Serial.print("Queue depth: ");
    Serial.println(m_metrics.queueDepth);
    Serial.println();
}
