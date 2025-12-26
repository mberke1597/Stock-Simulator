#ifndef PRICE_GENERATOR_H
#define PRICE_GENERATOR_H

#include "PriceData.h"
#include "SharedBuffer.h"
#include "PerformanceMonitor.h"
#include <thread>
#include <random>
#include <atomic>
#include <vector>
#include <iostream>

/**
 * @brief Producer Thread: Generates random stock price fluctuations
 * 
 * This class represents Thread 1 (Producer) in the multithreaded architecture.
 * It generates random price updates for multiple stock symbols at regular intervals
 * and pushes them to the shared buffer.
 * 
 * Threading Model:
 * - Runs in dedicated thread (std::thread)
 * - Uses atomic flag for thread-safe shutdown signaling
 * - Generates data independently from consumers
 */
class PriceGenerator {
private:
    SharedBuffer& buffer_;                      // Reference to shared buffer (synchronization point)
    PerformanceMonitor& perf_monitor_;         // Performance tracking
    std::vector<std::string> symbols_;          // Stock symbols to generate
    std::vector<double> current_prices_;        // Current price for each symbol
    
    std::atomic<bool> running_;                 // Thread-safe flag for shutdown
    std::thread thread_;                        // Worker thread
    
    int update_interval_ms_;                    // Time between price updates
    
    // Random number generation (thread-local)
    std::random_device rd_;
    std::mt19937 gen_;
    std::normal_distribution<> price_change_dist_;  // Normal distribution for realistic price changes

public:
    /**
     * @brief Constructor
     * @param buffer Shared buffer for inter-thread communication
     * @param perf_monitor Performance monitoring system
     * @param symbols List of stock symbols to simulate
     * @param update_interval_ms Milliseconds between price updates
     */
    PriceGenerator(SharedBuffer& buffer, 
                   PerformanceMonitor& perf_monitor,
                   const std::vector<std::string>& symbols = {"AAPL", "GOOGL", "MSFT", "AMZN", "BTC"},
                   int update_interval_ms = 100)
        : buffer_(buffer), perf_monitor_(perf_monitor), symbols_(symbols),
          running_(false), update_interval_ms_(update_interval_ms),
          gen_(rd_()), price_change_dist_(0.0, 0.5)  // Mean=0, StdDev=0.5 for price changes
    {
        // Initialize starting prices
        current_prices_.resize(symbols_.size());
        std::uniform_real_distribution<> init_price_dist(100.0, 500.0);
        for (auto& price : current_prices_) {
            price = init_price_dist(gen_);
        }
    }
    
    /**
     * @brief Start the producer thread
     * 
     * Launches a new thread that runs the generation loop.
     * Thread-safe: Uses atomic flag to prevent multiple starts.
     */
    void start() {
        bool expected = false;
        if (running_.compare_exchange_strong(expected, true)) {
            thread_ = std::thread(&PriceGenerator::run, this);
            std::cout << "[PriceGenerator] Started producer thread (ID: " 
                      << thread_.get_id() << ")\n";
        }
    }
    
    /**
     * @brief Stop the producer thread and wait for completion
     * 
     * Thread-safe shutdown:
     * 1. Sets atomic flag to signal thread to exit
     * 2. Joins thread to ensure clean termination
     */
    void stop() {
        if (running_.exchange(false)) {
            if (thread_.joinable()) {
                thread_.join();
                std::cout << "[PriceGenerator] Producer thread stopped\n";
            }
        }
    }
    
    /**
     * @brief Destructor ensures thread is properly stopped
     */
    ~PriceGenerator() {
        stop();
    }

private:
    /**
     * @brief Main producer loop (runs in separate thread)
     * 
     * Producer responsibilities:
     * 1. Generate random price fluctuations
     * 2. Create PriceData objects with high-resolution timestamps
     * 3. Push data to shared buffer (synchronization via mutex)
     * 4. Record performance metrics
     * 5. Sleep to control update rate
     * 
     * Synchronization:
     * - Uses SharedBuffer::push() which internally acquires mutex
     * - No explicit locking needed here (encapsulated in SharedBuffer)
     */
    void run() {
        std::cout << "[PriceGenerator] Producer loop starting...\n";
        
        size_t iteration = 0;
        
        while (running_.load()) {  // Check atomic flag
            auto generation_start = std::chrono::high_resolution_clock::now();
            
            // Generate price updates for all symbols
            for (size_t i = 0; i < symbols_.size(); ++i) {
                // Generate random price change using normal distribution
                double change = price_change_dist_(gen_);
                current_prices_[i] += change;
                
                // Ensure price doesn't go negative
                if (current_prices_[i] < 1.0) {
                    current_prices_[i] = 1.0;
                }
                
                // Create price data with high-resolution timestamp
                PriceData data(symbols_[i], current_prices_[i], change);
                
                // Push to shared buffer (CRITICAL SECTION handled internally)
                // This will acquire mutex, update buffer, release mutex, and notify consumers
                buffer_.push(data);
                
                // Record performance metric
                perf_monitor_.recordGeneration(symbols_[i], data.timestamp);
            }
            
            auto generation_end = std::chrono::high_resolution_clock::now();
            auto generation_time = std::chrono::duration_cast<std::chrono::microseconds>(
                generation_end - generation_start).count();
            
            ++iteration;
            
            // Log every 50 iterations to avoid cluttering console
            if (iteration % 50 == 0) {
                std::cout << "[PriceGenerator] Iteration " << iteration 
                          << " - Generation time: " << generation_time << " μs\n";
            }
            
            // Sleep to control update rate
            std::this_thread::sleep_for(std::chrono::milliseconds(update_interval_ms_));
        }
        
        std::cout << "[PriceGenerator] Producer loop exited after " 
                  << iteration << " iterations\n";
    }
};

#endif // PRICE_GENERATOR_H
