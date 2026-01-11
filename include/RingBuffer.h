/**
 * @file RingBuffer.h
 * @author chensong
 * @date 2026-01-11
 * @brief 高性能环形缓冲区模板类（Lock-Free SPSC Ring Buffer）
 * 
 * 该模块实现了无锁的单生产者单消费者（SPSC）环形缓冲区，
 * 适用于高性能视频帧传递场景。
 * 
 * 环形缓冲区架构（Ring Buffer Architecture）：
 * 
 *   ┌───────────────────────────────────────────────────────────┐
 *   │                    Ring Buffer (Size = 8)                  │
 *   ├───────────────────────────────────────────────────────────┤
 *   │                                                            │
 *   │   Index:   0     1     2     3     4     5     6     7     │
 *   │          ┌───┐ ┌───┐ ┌───┐ ┌───┐ ┌───┐ ┌───┐ ┌───┐ ┌───┐  │
 *   │          │   │ │ X │ │ X │ │ X │ │   │ │   │ │   │ │   │  │
 *   │          └───┘ └───┘ └───┘ └───┘ └───┘ └───┘ └───┘ └───┘  │
 *   │            ▲                       ▲                       │
 *   │            │                       │                       │
 *   │         tail=1                  head=4                     │
 *   │         (消费者读取位置)         (生产者写入位置)           │
 *   │                                                            │
 *   │   数据区间: [tail, head) = {1, 2, 3}                       │
 *   │   可用空间: Size - (head - tail) - 1 = 8 - 3 - 1 = 4       │
 *   │                                                            │
 *   └───────────────────────────────────────────────────────────┘
 * 
 * 无锁设计原理（Lock-Free Design Principles）：
 * 
 *   Producer (生产者)                Consumer (消费者)
 *   ─────────────────                ─────────────────
 *        │                                  │
 *        ▼                                  ▼
 *   ┌─────────────┐                  ┌─────────────┐
 *   │ load(tail)  │ ◄─── acquire ─── │ load(head)  │
 *   │ relaxed     │                  │ relaxed     │
 *   └─────────────┘                  └─────────────┘
 *        │                                  │
 *        ▼                                  ▼
 *   ┌─────────────┐                  ┌─────────────┐
 *   │ write data  │                  │ read data   │
 *   │ buffer[head]│                  │ buffer[tail]│
 *   └─────────────┘                  └─────────────┘
 *        │                                  │
 *        ▼                                  ▼
 *   ┌─────────────┐                  ┌─────────────┐
 *   │ store(head) │ ─── release ───► │ store(tail) │
 *   │ release     │                  │ release     │
 *   └─────────────┘                  └─────────────┘
 * 
 * 性能对比（Performance Comparison）：
 * 
 *   ┌───────────────────┬──────────────┬──────────────┐
 *   │     Operation     │  std::queue  │  RingBuffer  │
 *   ├───────────────────┼──────────────┼──────────────┤
 *   │ Push              │ mutex lock   │ atomic store │
 *   │ Pop               │ mutex lock   │ atomic load  │
 *   │ Memory allocation │ per element  │ pre-alloc    │
 *   │ Cache locality    │ poor         │ excellent    │
 *   │ Latency           │ ~100ns       │ ~10ns        │
 *   └───────────────────┴──────────────┴──────────────┘
 * 
 * @note 该实现仅适用于单生产者单消费者场景
 * @note 多生产者或多消费者场景需要额外同步机制
 * @see GpuFrameRingBuffer
 */

#pragma once

#include <atomic>
#include <array>
#include <cstddef>
#include <type_traits>

/**
 * @class RingBuffer
 * @brief 无锁单生产者单消费者环形缓冲区
 * 
 * @tparam T 元素类型
 * @tparam Capacity 缓冲区容量（必须是2的幂，用于位运算优化取模）
 * 
 * 使用示例：
 * @code
 * RingBuffer<int, 8> buffer;
 * 
 * // 生产者线程
 * if (buffer.push(42)) {
 *     // 写入成功
 * }
 * 
 * // 消费者线程
 * int value;
 * if (buffer.pop(value)) {
 *     // 读取成功
 * }
 * @endcode
 */
template<typename T, size_t Capacity>
class RingBuffer {
    // 确保容量是2的幂
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");
    static_assert(Capacity >= 2, "Capacity must be at least 2");

public:
    /**
     * @brief 构造函数
     */
    RingBuffer() : head_(0), tail_(0) {}

    /**
     * @brief 尝试将元素推入缓冲区（生产者调用）
     * 
     * @param item 要推入的元素（移动语义）
     * @return true 推入成功
     * @return false 缓冲区已满
     * 
     * @note 无锁操作，使用 release 语义确保写入对消费者可见
     */
    bool push(T&& item) {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        const size_t next_head = increment(current_head);
        
        // 检查是否有空间
        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false; // 缓冲区已满
        }
        
        // 写入数据
        buffer_[current_head] = std::move(item);
        
        // 更新 head（release 语义确保写入对消费者可见）
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    /**
     * @brief 尝试将元素推入缓冲区（左值版本）
     * 
     * @param item 要推入的元素（拷贝）
     * @return true 推入成功
     * @return false 缓冲区已满
     */
    bool push(const T& item) {
        T copy = item;
        return push(std::move(copy));
    }

    /**
     * @brief 尝试从缓冲区弹出元素（消费者调用）
     * 
     * @param item 输出参数，弹出的元素
     * @return true 弹出成功
     * @return false 缓冲区为空
     * 
     * @note 无锁操作，使用 acquire 语义确保读取到生产者写入的数据
     */
    bool pop(T& item) {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);
        
        // 检查是否有数据
        if (current_tail == head_.load(std::memory_order_acquire)) {
            return false; // 缓冲区为空
        }
        
        // 读取数据
        item = std::move(buffer_[current_tail]);
        
        // 更新 tail（release 语义确保读取完成后再更新）
        tail_.store(increment(current_tail), std::memory_order_release);
        return true;
    }

    /**
     * @brief 查看队首元素但不移除
     * 
     * @param item 输出参数
     * @return true 有元素
     * @return false 缓冲区为空
     */
    bool peek(T& item) const {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);
        
        if (current_tail == head_.load(std::memory_order_acquire)) {
            return false;
        }
        
        item = buffer_[current_tail];
        return true;
    }

    /**
     * @brief 检查缓冲区是否为空
     * @return true 为空
     * @return false 非空
     */
    bool empty() const {
        return head_.load(std::memory_order_acquire) == 
               tail_.load(std::memory_order_acquire);
    }

    /**
     * @brief 检查缓冲区是否已满
     * @return true 已满
     * @return false 未满
     */
    bool full() const {
        return increment(head_.load(std::memory_order_acquire)) == 
               tail_.load(std::memory_order_acquire);
    }

    /**
     * @brief 获取当前元素数量
     * @return size_t 元素数量
     */
    size_t size() const {
        const size_t head = head_.load(std::memory_order_acquire);
        const size_t tail = tail_.load(std::memory_order_acquire);
        return (head - tail) & (Capacity - 1);
    }

    /**
     * @brief 获取缓冲区容量
     * @return size_t 容量（实际可用容量为 Capacity - 1）
     */
    static constexpr size_t capacity() {
        return Capacity - 1; // 环形缓冲区需要保留一个空位
    }

    /**
     * @brief 清空缓冲区
     * @note 非线程安全，仅在单线程环境或同步后使用
     */
    void clear() {
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

private:
    /**
     * @brief 索引递增（使用位运算优化取模）
     * @param index 当前索引
     * @return size_t 下一个索引
     */
    static constexpr size_t increment(size_t index) {
        return (index + 1) & (Capacity - 1);
    }

    alignas(64) std::atomic<size_t> head_;  ///< 写入位置（生产者更新）
    alignas(64) std::atomic<size_t> tail_;  ///< 读取位置（消费者更新）
    std::array<T, Capacity> buffer_;         ///< 数据缓冲区
};

/**
 * @class BlockingRingBuffer
 * @brief 带阻塞等待的环形缓冲区（用于多生产者/多消费者场景）
 * 
 * 在 RingBuffer 基础上添加互斥锁和条件变量，支持阻塞等待操作。
 * 
 * @tparam T 元素类型
 * @tparam Capacity 缓冲区容量（必须是2的幂）
 */
template<typename T, size_t Capacity>
class BlockingRingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");

public:
    BlockingRingBuffer() : head_(0), tail_(0) {}

    /**
     * @brief 推入元素（可阻塞等待）
     * 
     * @param item 要推入的元素
     * @param wait 是否等待有空间
     * @return true 成功
     * @return false 缓冲区满且不等待
     */
    bool push(T&& item, bool wait = false) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (wait) {
            not_full_.wait(lock, [this] { return !is_full_unlocked(); });
        } else if (is_full_unlocked()) {
            return false;
        }
        
        buffer_[head_] = std::move(item);
        head_ = increment(head_);
        
        lock.unlock();
        not_empty_.notify_one();
        return true;
    }

    /**
     * @brief 弹出元素（可阻塞等待）
     * 
     * @param item 输出参数
     * @param wait 是否等待有数据
     * @return true 成功
     * @return false 缓冲区空且不等待
     */
    bool pop(T& item, bool wait = false) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (wait) {
            not_empty_.wait(lock, [this] { return !is_empty_unlocked(); });
        } else if (is_empty_unlocked()) {
            return false;
        }
        
        item = std::move(buffer_[tail_]);
        tail_ = increment(tail_);
        
        lock.unlock();
        not_full_.notify_one();
        return true;
    }

    /**
     * @brief 带超时的弹出
     * 
     * @param item 输出参数
     * @param timeout_ms 超时毫秒数
     * @return true 成功
     * @return false 超时或失败
     */
    bool pop_timeout(T& item, int timeout_ms) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (!not_empty_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                                  [this] { return !is_empty_unlocked(); })) {
            return false; // 超时
        }
        
        item = std::move(buffer_[tail_]);
        tail_ = increment(tail_);
        
        lock.unlock();
        not_full_.notify_one();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return is_empty_unlocked();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return (head_ - tail_) & (Capacity - 1);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = tail_ = 0;
    }

    static constexpr size_t capacity() {
        return Capacity - 1;
    }

private:
    bool is_empty_unlocked() const {
        return head_ == tail_;
    }

    bool is_full_unlocked() const {
        return increment(head_) == tail_;
    }

    static constexpr size_t increment(size_t index) {
        return (index + 1) & (Capacity - 1);
    }

    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    
    size_t head_ = 0;
    size_t tail_ = 0;
    std::array<T, Capacity> buffer_;
};

