#ifndef DISPLAY_THREAD_H
#define DISPLAY_THREAD_H

#include "PriceData.h"
#include "SharedBuffer.h"
#include <thread>
#include <atomic>
#include <iostream>
#include <iomanip>
#include <sstream>

// Periodically prints the latest prices to the console.
class DisplayThread {
private:
    SharedBuffer& buffer_;                  // Reference to shared buffer
    std::atomic<bool> running_;             // Thread-safe shutdown flag
    std::thread thread_;                    // Worker thread
    int refresh_interval_ms_;               // Display refresh rate

public:
    DisplayThread(SharedBuffer& buffer, int refresh_interval_ms = 500)
        : buffer_(buffer), running_(false), refresh_interval_ms_(refresh_interval_ms) {}
    
    void start() {
        bool expected = false;
        if (running_.compare_exchange_strong(expected, true)) {
            thread_ = std::thread(&DisplayThread::run, this);
            std::cout << "[DisplayThread] Started display thread (ID: " 
                      << thread_.get_id() << ")\n";
        }
    }
    
    void stop() {
        if (running_.exchange(false)) {
            if (thread_.joinable()) {
                thread_.join();
                std::cout << "[DisplayThread] Display thread stopped\n";
            }
        }
    }
    
    ~DisplayThread() {
        stop();
    }

private:
    void run() {
        std::cout << "[DisplayThread] Display loop starting...\n";
        std::cout << "\n========== REAL-TIME STOCK PRICE MONITOR ==========\n\n";
        
        while (running_.load()) {
            // Wait for new data with condition variable (efficient blocking)
            // This avoids busy-waiting and saves CPU cycles
            buffer_.waitForData(refresh_interval_ms_);
            
            if (!running_.load()) break;
            
            // Get all symbols being tracked
            auto symbols = buffer_.getSymbols();
            
            if (symbols.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            // Build display output
            std::stringstream output;
            output << "\r";  // Carriage return for in-place update
            output << std::fixed << std::setprecision(2);
            
            // Display each symbol's latest price
            for (const auto& symbol : symbols) {
                PriceData data;
                
                // Read from shared buffer (CRITICAL SECTION handled internally)
                if (buffer_.getLatest(symbol, data)) {
                    // ASCII-only direction indicator for better Windows console compatibility
                    std::string indicator = (data.change >= 0) ? "UP" : "DN";
                    
                    output << symbol << ": $" << std::setw(8) << data.price 
                           << " " << indicator << " " 
                           << std::setw(6) << std::showpos << data.change 
                           << std::noshowpos << " | ";
                }
            }
            
            // Print to console (non-blocking write)
            std::cout << output.str() << std::flush;
            
            // Sleep to control refresh rate
            std::this_thread::sleep_for(std::chrono::milliseconds(refresh_interval_ms_));
        }
        
        std::cout << "\n[DisplayThread] Display loop exited\n";
    }
};

#endif // DISPLAY_THREAD_H
