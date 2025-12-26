#ifndef SHARED_BUFFER_H
#define SHARED_BUFFER_H

#include "PriceData.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <map>
#include <deque>

/**
 * @brief Thread-safe circular buffer for price data
 * 
 * This class implements a thread-safe shared memory structure using:
 * - std::mutex for mutual exclusion (prevents race conditions)
 * - std::condition_variable for producer-consumer synchronization
 * - Circular buffer pattern for efficient memory usage
 * 
 * Design prevents deadlocks by:
 * 1. Consistent lock ordering (single mutex)
 * 2. Bounded wait times (notify_all to wake all waiting threads)
 * 3. No nested locks
 */
class SharedBuffer {
private:
    // Map of symbol -> deque of price history (circular buffer per symbol)
    std::map<std::string, std::deque<PriceData>> price_history_;
    
    // Maximum history size per symbol (circular buffer constraint)
    const size_t max_history_size_;
    
    // Synchronization primitives
    mutable std::mutex mutex_;              // Protects all shared data
    std::condition_variable cv_data_ready_; // Notifies consumers when new data arrives
    
    // Shutdown flag for graceful termination
    bool shutdown_;
    
    // Statistics
    size_t total_writes_;
    size_t total_reads_;

public:
    /**
     * @brief Constructor
     * @param max_size Maximum number of price ticks to store per symbol
     */
    explicit SharedBuffer(size_t max_size = 100) 
        : max_history_size_(max_size), shutdown_(false), 
          total_writes_(0), total_reads_(0) {}
    
    /**
     * @brief Producer: Add new price data (thread-safe)
     * 
     * Critical Section protected by mutex_:
     * - Modifies price_history_ (shared resource)
     * - Updates statistics
     * 
     * Notifies all waiting consumer threads via condition variable
     * after data is written (Producer-Consumer pattern).
     * 
     * @param data Price data to add
     */
    void push(const PriceData& data) {
        {
            std::unique_lock<std::mutex> lock(mutex_);  // RAII lock acquisition
            
            // Add to history for this symbol
            auto& history = price_history_[data.symbol];
            history.push_back(data);
            
            // Maintain circular buffer: remove oldest if exceeds max size
            if (history.size() > max_history_size_) {
                history.pop_front();
            }
            
            ++total_writes_;
        }  // Lock released here automatically (RAII)
        
        // Notify ALL waiting consumer threads that new data is available
        // This is done OUTSIDE the lock to avoid unnecessary blocking
        cv_data_ready_.notify_all();
    }
    
    /**
     * @brief Consumer: Get latest price for a symbol (thread-safe)
     * 
     * Critical Section protected by mutex_:
     * - Reads price_history_ (shared resource)
     * 
     * @param symbol Stock symbol to query
     * @param data Output parameter for price data
     * @return true if data found, false otherwise
     */
    bool getLatest(const std::string& symbol, PriceData& data) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        auto it = price_history_.find(symbol);
        if (it != price_history_.end() && !it->second.empty()) {
            data = it->second.back();  // Get most recent
            ++total_reads_;
            return true;
        }
        return false;
    }
    
    /**
     * @brief Consumer: Get price history for indicator calculations (thread-safe)
     * 
     * Critical Section protected by mutex_:
     * - Reads price_history_ (shared resource)
     * 
     * @param symbol Stock symbol to query
     * @param count Number of recent prices to retrieve
     * @return Vector of recent prices (oldest to newest)
     */
    std::vector<PriceData> getHistory(const std::string& symbol, size_t count) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        std::vector<PriceData> result;
        auto it = price_history_.find(symbol);
        
        if (it != price_history_.end()) {
            const auto& history = it->second;
            size_t start = (history.size() > count) ? (history.size() - count) : 0;
            
            for (size_t i = start; i < history.size(); ++i) {
                result.push_back(history[i]);
            }
            ++total_reads_;
        }
        
        return result;
    }
    
    /**
     * @brief Consumer: Wait for new data (blocking with condition variable)
     * 
     * Uses condition variable to efficiently wait for producer signal.
     * This avoids busy-waiting (polling) which wastes CPU cycles.
     * 
     * Demonstrates Producer-Consumer pattern:
     * - Consumers sleep until producer signals via cv_data_ready_
     * - Prevents race conditions via mutex protection
     * 
     * @param timeout_ms Maximum wait time in milliseconds
     * @return true if woken by signal, false if timeout
     */
    bool waitForData(int timeout_ms = 1000) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait until either:
        // 1. New data arrives (predicate becomes true)
        // 2. Timeout expires
        // 3. Shutdown signaled
        return cv_data_ready_.wait_for(lock, 
                                        std::chrono::milliseconds(timeout_ms),
                                        [this] { return shutdown_ || total_writes_ > 0; });
    }
    
    /**
     * @brief Get all tracked symbols (thread-safe)
     */
    std::vector<std::string> getSymbols() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        std::vector<std::string> symbols;
        for (const auto& pair : price_history_) {
            symbols.push_back(pair.first);
        }
        return symbols;
    }
    
    /**
     * @brief Signal shutdown to all waiting threads
     * 
     * Graceful shutdown mechanism:
     * - Sets shutdown flag
     * - Wakes all waiting threads so they can exit
     */
    void shutdown() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_data_ready_.notify_all();
    }
    
    /**
     * @brief Check if shutdown was signaled
     */
    bool isShutdown() {
        std::unique_lock<std::mutex> lock(mutex_);
        return shutdown_;
    }
    
    /**
     * @brief Get statistics (thread-safe)
     */
    void getStats(size_t& writes, size_t& reads) {
        std::unique_lock<std::mutex> lock(mutex_);
        writes = total_writes_;
        reads = total_reads_;
    }
};

#endif // SHARED_BUFFER_H
