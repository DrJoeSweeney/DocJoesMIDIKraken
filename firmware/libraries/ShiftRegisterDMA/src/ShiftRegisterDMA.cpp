#include "ShiftRegisterDMA.h"

#ifdef ESP32

ShiftRegisterDMA::ShiftRegisterDMA(spi_host_device_t host, int misoPin, int sckPin, int latchPin, uint8_t numBytes)
    : m_host(host)
    , m_spiHandle(nullptr)
    , m_misoPin(misoPin)
    , m_sckPin(sckPin)
    , m_latchPin(latchPin)
    , m_numBytes(numBytes)
    , m_dmaBuffer(nullptr)
    , m_lastTransferTime(0)
    , m_dmaInProgress(false)
{
    // Allocate DMA-capable buffer (must be 4-byte aligned)
    m_dmaBuffer = (uint8_t*)heap_caps_malloc(m_numBytes, MALLOC_CAP_DMA);
    memset(m_dmaBuffer, 0, m_numBytes);
}

ShiftRegisterDMA::~ShiftRegisterDMA() {
    if (m_spiHandle) {
        spi_bus_remove_device(m_spiHandle);
        spi_bus_free(m_host);
    }
    if (m_dmaBuffer) {
        heap_caps_free(m_dmaBuffer);
    }
}

bool ShiftRegisterDMA::begin(uint32_t clockSpeedHz) {
    // Configure latch pin
    pinMode(m_latchPin, OUTPUT);
    digitalWrite(m_latchPin, HIGH);

    // Configure SPI bus
    spi_bus_config_t busConfig = {
        .mosi_io_num = -1,              // Not used
        .miso_io_num = m_misoPin,
        .sclk_io_num = m_sckPin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = m_numBytes,
        .flags = 0,
        .intr_flags = 0
    };

    esp_err_t ret = spi_bus_initialize(m_host, &busConfig, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        return false;
    }

    // Configure SPI device
    spi_device_interface_config_t devConfig = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,                      // SPI mode 0 (CPOL=0, CPHA=0)
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = clockSpeedHz,
        .input_delay_ns = 0,
        .spics_io_num = -1,             // No CS pin (using latch pin)
        .flags = 0,
        .queue_size = 1,
        .pre_cb = nullptr,
        .post_cb = nullptr
    };

    ret = spi_bus_add_device(m_host, &devConfig, &m_spiHandle);
    if (ret != ESP_OK) {
        spi_bus_free(m_host);
        return false;
    }

    return true;
}

bool ShiftRegisterDMA::startDMA() {
    if (m_dmaInProgress) {
        return false;  // DMA already in progress
    }

    uint32_t startTime = micros();

    // Latch shift register inputs
    latch();

    // Prepare SPI transaction
    spi_transaction_t trans = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = m_numBytes * 8,       // Length in bits
        .rxlength = m_numBytes * 8,
        .user = nullptr,
        .tx_buffer = nullptr,
        .rx_buffer = m_dmaBuffer
    };

    // Start DMA transfer (non-blocking)
    esp_err_t ret = spi_device_queue_trans(m_spiHandle, &trans, 0);
    if (ret == ESP_OK) {
        m_dmaInProgress = true;
        m_lastTransferTime = micros() - startTime;
        return true;
    }

    return false;
}

bool ShiftRegisterDMA::isDMAComplete() {
    if (!m_dmaInProgress) {
        return true;  // No transfer in progress
    }

    spi_transaction_t* rtrans;
    esp_err_t ret = spi_device_get_trans_result(m_spiHandle, &rtrans, 0);

    if (ret == ESP_OK) {
        m_dmaInProgress = false;
        return true;
    }

    return false;  // Still in progress
}

bool ShiftRegisterDMA::waitForDMA(uint32_t timeoutMs) {
    if (!m_dmaInProgress) {
        return true;
    }

    spi_transaction_t* rtrans;
    esp_err_t ret = spi_device_get_trans_result(m_spiHandle, &rtrans, pdMS_TO_TICKS(timeoutMs));

    if (ret == ESP_OK) {
        m_dmaInProgress = false;
        return true;
    }

    return false;  // Timeout
}

const uint8_t* ShiftRegisterDMA::getDMABuffer() const {
    return m_dmaBuffer;
}

uint8_t ShiftRegisterDMA::getByte(uint8_t byteIndex) {
    if (byteIndex >= m_numBytes) {
        return 0;
    }
    return m_dmaBuffer[byteIndex];
}

bool ShiftRegisterDMA::getBit(uint16_t bitIndex) {
    uint8_t byteIndex = bitIndex / 8;
    uint8_t bitOffset = bitIndex % 8;

    if (byteIndex >= m_numBytes) {
        return false;
    }

    return (m_dmaBuffer[byteIndex] & (1 << bitOffset)) != 0;
}

void ShiftRegisterDMA::latch() {
    // Pulse latch LOW to load parallel inputs
    digitalWrite(m_latchPin, LOW);
    delayMicroseconds(1);
    digitalWrite(m_latchPin, HIGH);
    delayMicroseconds(1);
}

#endif // ESP32
