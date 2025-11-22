#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H

#include <Arduino.h>
#include <Protocol.h>

#ifdef ESP32
#include <atomic>

/**
 * LockFreeQueue - Thread-Safe Lock-Free Queue for ESP32
 *
 * Lock-free SPSC (Single Producer Single Consumer) queue for inter-core communication.
 * Core 0 (scanner) pushes EventMessage items, Core 1 (communication) pops them.
 *
 * Features:
 * - No locks or mutexes (zero contention)
 * - Atomic operations for thread safety
 * - Fixed-size ring buffer
 * - Wait-free push/pop operations
 *
 * Typical usage:
 *   LockFreeQueue<EventMessage> queue(128);
 *
 *   // Core 0 (scanner):
 *   EventMessage event = {...};
 *   if (queue.push(event)) { ... }
 *
 *   // Core 1 (communication):
 *   EventMessage event;
 *   if (queue.pop(event)) { ... }
 */
template<typename T>
class LockFreeQueue {
public:
    /**
     * Constructor
     * @param capacity - Maximum queue size (must be power of 2)
     */
    LockFreeQueue(uint32_t capacity)
        : m_capacity(capacity)
        , m_mask(capacity - 1)
        , m_head(0)
        , m_tail(0)
    {
        // Ensure capacity is power of 2
        if ((capacity & (capacity - 1)) != 0) {
            m_capacity = nextPowerOf2(capacity);
            m_mask = m_capacity - 1;
        }

        m_buffer = new T[m_capacity];
    }

    ~LockFreeQueue() {
        delete[] m_buffer;
    }

    /**
     * Push item to queue (producer)
     * @param item - Item to push
     * @return true if successful, false if queue full
     */
    bool push(const T& item) {
        uint32_t head = m_head.load(std::memory_order_relaxed);
        uint32_t nextHead = (head + 1) & m_mask;

        if (nextHead == m_tail.load(std::memory_order_acquire)) {
            return false;  // Queue full
        }

        m_buffer[head] = item;
        m_head.store(nextHead, std::memory_order_release);
        return true;
    }

    /**
     * Pop item from queue (consumer)
     * @param item - Output parameter for popped item
     * @return true if successful, false if queue empty
     */
    bool pop(T& item) {
        uint32_t tail = m_tail.load(std::memory_order_relaxed);

        if (tail == m_head.load(std::memory_order_acquire)) {
            return false;  // Queue empty
        }

        item = m_buffer[tail];
        uint32_t nextTail = (tail + 1) & m_mask;
        m_tail.store(nextTail, std::memory_order_release);
        return true;
    }

    /**
     * Check if queue is empty
     */
    bool isEmpty() const {
        return m_tail.load(std::memory_order_relaxed) == m_head.load(std::memory_order_relaxed);
    }

    /**
     * Check if queue is full
     */
    bool isFull() const {
        uint32_t head = m_head.load(std::memory_order_relaxed);
        uint32_t nextHead = (head + 1) & m_mask;
        return nextHead == m_tail.load(std::memory_order_relaxed);
    }

    /**
     * Get current queue size
     */
    uint32_t size() const {
        uint32_t head = m_head.load(std::memory_order_relaxed);
        uint32_t tail = m_tail.load(std::memory_order_relaxed);

        if (head >= tail) {
            return head - tail;
        } else {
            return m_capacity - (tail - head);
        }
    }

    /**
     * Get queue capacity
     */
    uint32_t capacity() const {
        return m_capacity;
    }

    /**
     * Clear queue (not thread-safe, use only when both threads stopped)
     */
    void clear() {
        m_head.store(0, std::memory_order_relaxed);
        m_tail.store(0, std::memory_order_relaxed);
    }

private:
    T* m_buffer;
    uint32_t m_capacity;
    uint32_t m_mask;

    std::atomic<uint32_t> m_head;  // Write by producer
    std::atomic<uint32_t> m_tail;  // Write by consumer

    /**
     * Round up to next power of 2
     */
    uint32_t nextPowerOf2(uint32_t n) {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
        return n;
    }
};

#endif // ESP32
#endif // LOCK_FREE_QUEUE_H
