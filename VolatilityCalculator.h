#ifndef VOLATILITY_CALCULATOR_H
#define VOLATILITY_CALCULATOR_H

#include "PriceData.h"
#include "SharedBuffer.h"
#include "PerformanceMonitor.h"
#include <thread>
#include <atomic>
#include <iostream>
#include <cmath>
#include <numeric>
#include <iomanip>

/**
 * @brief Consumer Thread 3: Volatility Indicator
 * 
 * This class represents Thread 4 (Consumer/Analytic) in the multithreaded architecture.
 * It calculates the volatility (standard deviation of returns) for each stock symbol.
 * 
 * Threading Model:
 * - Runs in dedicated thread
 * - Consumer in Producer-Consumer pattern
 * - Demonstrates concurrent analytical processing
 * - Independent from other consumer threads (no inter-consumer synchronization needed)
 */
class VolatilityCalculator {
private:
    SharedBuffer& buffer_;                  // Reference to shared buffer
    PerformanceMonitor& perf_monitor_;     // Performance tracking
    std::atomic<bool> running_;             // Thread-safe shutdown flag
    std::thread thread_;                    // Worker thread
    
    int calculation_interval_ms_;           // Time between calculations
    size_t window_size_;                    // Volatility window size

public:
    /**
     * @brief Constructor
     * @param buffer Shared buffer for reading price history
     * @param perf_monitor Performance monitoring system
     * @param window_size Number of periods for volatility calculation
     * @param calculation_interval_ms Milliseconds between calculations
     */
    VolatilityCalculator(SharedBuffer& buffer,
                         PerformanceMonitor& perf_monitor,
                         size_t window_size = 20,
                         int calculation_interval_ms = 1500)
        : buffer_(buffer), perf_monitor_(perf_monitor), running_(false),
          calculation_interval_ms_(calculation_interval_ms), window_size_(window_size) {}
    
    /**
     * @brief Start the volatility calculator thread
     */
    void start() {
        bool expected = false;
        if (running_.compare_exchange_strong(expected, true)) {
            thread_ = std::thread(&VolatilityCalculator::run, this);
            std::cout << "[VolatilityCalculator] Started volatility calculator thread (ID: " 
                      << thread_.get_id() << ") with window size " << window_size_ << "\n";
        }
    }
    
    /**
     * @brief Stop the volatility calculator thread
     */
    void stop() {
        if (running_.exchange(false)) {
            if (thread_.joinable()) {
                thread_.join();
                std::cout << "[VolatilityCalculator] Volatility calculator thread stopped\n";
            }
        }
    }
    
    /**
     * @brief Destructor
     */
    ~VolatilityCalculator() {
        stop();
    }

private:
    /**
     * @brief Main calculation loop (runs in separate thread)
     * 
     * Consumer responsibilities:
     * 1. Wait for new data (condition variable)
     * 2. Read price history from shared buffer (mutex protection)
     * 3. Calculate volatility (standard deviation of returns)
     * 4. Record performance metrics
     * 5. Log results
     * 
     * Volatility Formula:
     * - Calculate returns: (P[i] - P[i-1]) / P[i-1]
     * - Calculate standard deviation of returns
     * - Annualize: volatility * sqrt(252 trading days)
     * 
     * Synchronization:
     * - Uses SharedBuffer methods with internal mutex protection
     * - No deadlock risk: single lock acquisition per operation
     * - Thread-safe via encapsulated synchronization
     */
    void run() {
        std::cout << "[VolatilityCalculator] Volatility calculation loop starting...\n";
        
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
            
            // Calculate volatility for each symbol
            for (const auto& symbol : symbols) {
                // Read price history (CRITICAL SECTION handled internally)
                auto history = buffer_.getHistory(symbol, window_size_);
                
                if (history.size() < 3) {
                    continue;  // Need at least 3 data points
                }
                
                // Calculate returns
                std::vector<double> returns;
                for (size_t i = 1; i < history.size(); ++i) {
                    double return_pct = (history[i].price - history[i-1].price) / history[i-1].price;
                    returns.push_back(return_pct);
                }
                
                // Calculate mean return
                double mean_return = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
                
                // Calculate variance
                double variance = 0.0;
                for (double ret : returns) {
                    variance += (ret - mean_return) * (ret - mean_return);
                }
                variance /= returns.size();
                
                // Calculate standard deviation (volatility)
                double volatility = std::sqrt(variance);
                
                // Annualized volatility (assuming 252 trading days)
                double annualized_volatility = volatility * std::sqrt(252.0) * 100.0;  // As percentage
                
                // Record performance: latency from generation to calculation
                auto generation_time = history.back().timestamp;
                auto processing_time = std::chrono::high_resolution_clock::now();
                perf_monitor_.recordProcessing(symbol, "Volatility", generation_time, processing_time);
                
                ++calculation_count;
                
                // Classify volatility level
                std::string volatility_level;
                if (annualized_volatility < 15.0) {
                    volatility_level = "LOW";
                } else if (annualized_volatility < 30.0) {
                    volatility_level = "MODERATE";
                } else {
                    volatility_level = "HIGH";
                }
                
                // Log results periodically
                if (calculation_count % 15 == 0) {
                    std::cout << "\n[VolatilityCalculator] " << symbol 
                              << " - Price: $" << std::fixed << std::setprecision(2) << history.back().price
                              << " | Volatility: " << std::setprecision(2) << annualized_volatility << "% (annualized)"
                              << " | Level: " << volatility_level
                              << " | Sample size: " << returns.size();
                }
            }
            
            auto calc_end = std::chrono::high_resolution_clock::now();
            auto calc_time = std::chrono::duration_cast<std::chrono::microseconds>(
                calc_end - calc_start).count();
            
            // Log calculation performance
            if (calculation_count % 15 == 0) {
                std::cout << " | Calc time: " << calc_time << " μs\n";
            }
            
            // Sleep to control calculation rate
            std::this_thread::sleep_for(std::chrono::milliseconds(calculation_interval_ms_));
        }
        
        std::cout << "\n[VolatilityCalculator] Volatility calculation loop exited after " 
                  << calculation_count << " calculations\n";
    }
};

#endif // VOLATILITY_CALCULATOR_H
