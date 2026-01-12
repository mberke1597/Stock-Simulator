#ifndef SMA_CALCULATOR_H
#define SMA_CALCULATOR_H

#include "PriceData.h"
#include "SharedBuffer.h"
#include "PerformanceMonitor.h"
#include <thread>
#include <atomic>
#include <iostream>
#include <numeric>
#include <iomanip>

// Background thread that keeps a rolling SMA per symbol.
class SMACalculator {
private:
    SharedBuffer& buffer_;                  // Reference to shared buffer
    PerformanceMonitor& perf_monitor_;     // Performance tracking
    std::atomic<bool> running_;             // Thread-safe shutdown flag
    std::thread thread_;                    // Worker thread
    
    int calculation_interval_ms_;           // Time between calculations
    size_t window_size_;                    // SMA window size (e.g., 20 periods)

public:
    SMACalculator(SharedBuffer& buffer, 
                  PerformanceMonitor& perf_monitor,
                  size_t window_size = 20,
                  int calculation_interval_ms = 1000)
        : buffer_(buffer), perf_monitor_(perf_monitor), running_(false),
          calculation_interval_ms_(calculation_interval_ms), window_size_(window_size) {}
    
    void start() {
        bool expected = false;
        if (running_.compare_exchange_strong(expected, true)) {
            thread_ = std::thread(&SMACalculator::run, this);
            std::cout << "[SMACalculator] Started SMA calculator thread (ID: " 
                      << thread_.get_id() << ") with window size " << window_size_ << "\n";
        }
    }
    
    void stop() {
        if (running_.exchange(false)) {
            if (thread_.joinable()) {
                thread_.join();
                std::cout << "[SMACalculator] SMA calculator thread stopped\n";
            }
        }
    }
    
    ~SMACalculator() {
        stop();
    }

private:
    void run() {
        std::cout << "[SMACalculator] SMA calculation loop starting...\n";
        
        size_t calculation_count = 0;
        
        while (running_.load()) {
            // Wait for new data with condition variable
            buffer_.waitForData(calculation_interval_ms_);
            
            if (!running_.load()) break;
            
            // Get all symbols being tracked
            auto symbols = buffer_.getSymbols();
            
            if (symbols.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            auto calc_start = std::chrono::high_resolution_clock::now();
            
            // Calculate SMA for each symbol
            for (const auto& symbol : symbols) {
                // Read price history (CRITICAL SECTION handled internally)
                auto history = buffer_.getHistory(symbol, window_size_);
                
                if (history.size() < 2) {
                    continue;  // Not enough data yet
                }
                
                // Calculate Simple Moving Average
                double sum = 0.0;
                for (const auto& data : history) {
                    sum += data.price;
                }
                double sma = sum / history.size();
                
                // Get latest price for comparison
                double latest_price = history.back().price;
                double deviation = ((latest_price - sma) / sma) * 100.0;
                
                // Record performance: latency from generation to calculation
                auto generation_time = history.back().timestamp;
                auto processing_time = std::chrono::high_resolution_clock::now();
                perf_monitor_.recordProcessing(symbol, "SMA", generation_time, processing_time);
                
                ++calculation_count;
                
                // Log results periodically
                if (calculation_count % 20 == 0) {
                    std::cout << "\n[SMACalculator] " << symbol 
                              << " - Price: $" << std::fixed << std::setprecision(2) << latest_price
                              << " | SMA(" << history.size() << "): $" << sma
                              << " | Deviation: " << std::showpos << deviation << "%" << std::noshowpos;
                }
            }
            
            auto calc_end = std::chrono::high_resolution_clock::now();
            auto calc_time = std::chrono::duration_cast<std::chrono::microseconds>(
                calc_end - calc_start).count();
            
            // Log calculation performance (display in microseconds using ASCII)
            if (calculation_count % 20 == 0) {
                std::cout << " | Calc time: " << calc_time << " us\n";
            }
            
            // Sleep to control calculation rate
            std::this_thread::sleep_for(std::chrono::milliseconds(calculation_interval_ms_));
        }
        
        std::cout << "\n[SMACalculator] SMA calculation loop exited after " 
                  << calculation_count << " calculations\n";
    }
};

#endif // SMA_CALCULATOR_H
