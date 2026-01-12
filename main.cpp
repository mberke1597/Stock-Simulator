// Entry point for the real-time stock price simulator.

#include "SharedBuffer.h"
#include "PriceGenerator.h"
#include "DisplayThread.h"
#include "SMACalculator.h"
#include "VolatilityCalculator.h"
#include "PerformanceMonitor.h"

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

// Global flag for graceful shutdown on Ctrl+C
std::atomic<bool> g_shutdown_requested(false);

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n\n[Main] Shutdown signal received (Ctrl+C)...\n";
        g_shutdown_requested.store(true);
    }
}

/**
 * @brief Main application entry point
 * 
 * Responsibilities:
 * 1. Initialize shared resources (buffer, performance monitor)
 * 2. Create and start all threads
 * 3. Monitor for shutdown signal
 * 4. Coordinate graceful shutdown
 * 5. Display performance report
 */
int main(int argc, char* argv[]) {
    // Set up signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);
    
    std::cout << "========================================================\n";
    std::cout << "    REAL-TIME STOCK PRICE SIMULATOR (Multithreaded)    \n";
    std::cout << "========================================================\n";
    std::cout << "\nOperating Systems Concepts Demonstrated:\n";
    std::cout << "  - Multithreading (std::thread)\n";
    std::cout << "  - Mutex Synchronization (std::mutex)\n";
    std::cout << "  - Condition Variables (std::condition_variable)\n";
    std::cout << "  - Producer-Consumer Pattern\n";
    std::cout << "  - Thread-Safe Shared Memory (Circular Buffer)\n";
    std::cout << "  - Deadlock-Free Design\n";
    std::cout << "  - High-Resolution Performance Measurement\n";
    std::cout << "\n========================================================\n\n";
    
    // Parse command-line arguments (optional: runtime duration)
    int runtime_seconds = 45;  // Default: 45 seconds
    if (argc > 1) {
        try {
            runtime_seconds = std::stoi(argv[1]);
            if (runtime_seconds < 5 || runtime_seconds > 300) {
                std::cerr << "Runtime must be between 5 and 300 seconds. Using default (30s).\n";
                runtime_seconds = 30;
            }
        } catch (...) {
            std::cerr << "Invalid runtime argument. Using default (30s).\n";
        }
    }
    
    std::cout << "[Main] Simulation will run for " << runtime_seconds << " seconds\n";
    std::cout << "[Main] Press Ctrl+C to stop early and view performance report\n\n";
    
    // ============================================================
    // STEP 1: Initialize shared resources
    // ============================================================
    
    std::cout << "[Main] Initializing shared resources...\n";
    
    // Thread-safe circular buffer (max 100 price ticks per symbol)
    SharedBuffer shared_buffer(100);
    
    // Performance monitoring system
    PerformanceMonitor perf_monitor;
    
    // Stock symbols to simulate
    std::vector<std::string> symbols = {"AAPL", "GOOGL", "MSFT", "AMZN", "BTC"};
    
    std::cout << "[Main] Tracking symbols: ";
    for (const auto& sym : symbols) {
        std::cout << sym << " ";
    }
    std::cout << "\n\n";
    
    // ============================================================
    // STEP 2: Create thread objects
    // ============================================================
    
    std::cout << "[Main] Creating thread objects...\n";
    
    // Thread 1: Producer (Price Generator)
    // Generates random prices every 100ms
    PriceGenerator price_generator(shared_buffer, perf_monitor, symbols, 100);
    
    // Thread 2: Consumer (Display)
    // Updates console display every 500ms
    DisplayThread display_thread(shared_buffer, 500);
    
    // Thread 3: Consumer (SMA Calculator)
    // Calculates 20-period moving average every 1000ms
    SMACalculator sma_calculator(shared_buffer, perf_monitor, 20, 1000);
    
    // Thread 4: Consumer (Volatility Calculator)
    // Calculates volatility every 1500ms
    VolatilityCalculator volatility_calculator(shared_buffer, perf_monitor, 20, 1500);
    
    // ============================================================
    // STEP 3: Start all threads
    // ============================================================
    
    std::cout << "\n[Main] Starting all threads...\n\n";
    
    // Start producer
    price_generator.start();
    
    // Small delay to ensure some data is generated before consumers start
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Start consumers
    display_thread.start();
    sma_calculator.start();
    volatility_calculator.start();
    
    std::cout << "\n[Main] All threads running. Monitoring system...\n";
    std::cout << "[Main] System will automatically stop after " << runtime_seconds << " seconds\n\n";
    
    // ============================================================
    // STEP 4: Monitor for shutdown
    // ============================================================
    
    // Main thread monitors for shutdown signal or timeout
    auto start_time = std::chrono::steady_clock::now();
    
    while (!g_shutdown_requested.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Check if runtime exceeded
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - start_time).count();
        
        if (elapsed >= runtime_seconds) {
            std::cout << "\n\n[Main] Runtime limit reached (" << runtime_seconds << "s)\n";
            break;
        }
    }
    
    // ============================================================
    // STEP 5: Graceful shutdown
    // ============================================================
    
    std::cout << "[Main] Initiating graceful shutdown...\n\n";
    
    // Signal shared buffer to wake waiting threads
    shared_buffer.shutdown();
    
    // Stop all threads (joins will block until threads exit)
    std::cout << "[Main] Stopping producer thread...\n";
    price_generator.stop();
    
    std::cout << "[Main] Stopping consumer threads...\n";
    display_thread.stop();
    sma_calculator.stop();
    volatility_calculator.stop();
    
    std::cout << "\n[Main] All threads stopped successfully\n";
    
    // ============================================================
    // STEP 6: Display performance report
    // ============================================================
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    perf_monitor.printReport();
    
    // Additional statistics
    size_t total_writes, total_reads;
    shared_buffer.getStats(total_writes, total_reads);
    
    std::cout << "--- Shared Buffer Statistics ---\n";
    std::cout << "Total Writes: " << total_writes << "\n";
    std::cout << "Total Reads: " << total_reads << "\n";
    std::cout << "Read/Write Ratio: " << std::fixed << std::setprecision(2) 
              << (static_cast<double>(total_reads) / total_writes) << "\n\n";
    
    std::cout << "====================================================\n";
    std::cout << "     Simulation completed successfully!             \n";
    std::cout << "====================================================\n\n";
    
    std::cout << "Thank you for using the Real-Time Stock Price Simulator!\n";
    std::cout << "This project demonstrates key Operating Systems concepts.\n\n";
    
    return 0;
}
