#ifndef SHIFT_REGISTER_DMA_H
#define SHIFT_REGISTER_DMA_H

#include <Arduino.h>

#ifdef ESP32
#include <driver/spi_master.h>

/**
 * ShiftRegisterDMA - DMA-Accelerated 74HC165 Reader for ESP32
 *
 * Uses ESP32 SPI peripheral with DMA for hardware-accelerated shift register reading.
 * Significantly faster than bit-banging with zero CPU overhead during transfer.
 *
 * Features:
 * - DMA-based SPI reading (no CPU involvement)
 * - Non-blocking operation
 * - 10MHz+ clock speeds possible
 * - ~5µs for 16 bytes (128 bits) on ESP32
 *
 * Typical usage:
 *   ShiftRegisterDMA sr(VSPI, MISO_PIN, SCK_PIN, LATCH_PIN, 16);
 *   sr.begin();
 *   sr.startDMA();
 *   while (!sr.isDMAComplete()) { taskYIELD(); }
 *   const uint8_t* data = sr.getDMABuffer();
 *
 * Note: ESP32-specific using SPI peripheral
 */
class ShiftRegisterDMA {
public:
    /**
     * Constructor
     * @param host - SPI host (VSPI_HOST or HSPI_HOST)
     * @param misoPin - MISO pin (serial data from shift registers)
     * @param sckPin - SCK pin (clock)
     * @param latchPin - Latch/Load pin (SH/LD)
     * @param numBytes - Number of bytes to read
     */
    ShiftRegisterDMA(spi_host_device_t host, int misoPin, int sckPin, int latchPin, uint8_t numBytes);
    ~ShiftRegisterDMA();

    /**
     * Initialize SPI and DMA
     * @param clockSpeedHz - SPI clock speed (default 1MHz)
     * @return true if successful
     */
    bool begin(uint32_t clockSpeedHz = 1000000);

    /**
     * Start DMA read operation (non-blocking)
     * @return true if DMA started successfully
     */
    bool startDMA();

    /**
     * Check if DMA read is complete
     * @return true if DMA transfer finished
     */
    bool isDMAComplete();

    /**
     * Wait for DMA to complete (blocking)
     * @param timeoutMs - Timeout in milliseconds
     * @return true if completed, false if timeout
     */
    bool waitForDMA(uint32_t timeoutMs = 100);

    /**
     * Get pointer to DMA buffer (read-only)
     * Call this after DMA is complete
     * @return Pointer to buffer containing shift register data
     */
    const uint8_t* getDMABuffer() const;

    /**
     * Get a specific byte from buffer
     * @param byteIndex - Byte index
     * @return Byte value
     */
    uint8_t getByte(uint8_t byteIndex);

    /**
     * Get a specific bit from buffer
     * @param bitIndex - Bit index
     * @return true if bit is HIGH
     */
    bool getBit(uint16_t bitIndex);

    /**
     * Get last DMA transfer time (microseconds)
     */
    uint32_t getLastTransferTime() const { return m_lastTransferTime; }

private:
    spi_host_device_t m_host;
    spi_device_handle_t m_spiHandle;
    int m_misoPin;
    int m_sckPin;
    int m_latchPin;
    uint8_t m_numBytes;
    uint8_t* m_dmaBuffer;
    uint32_t m_lastTransferTime;
    bool m_dmaInProgress;

    void latch();
};

#endif // ESP32
#endif // SHIFT_REGISTER_DMA_H
