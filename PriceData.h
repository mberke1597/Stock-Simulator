#ifndef PRICE_DATA_H
#define PRICE_DATA_H

#include <string>
#include <chrono>

// Represents a single price update with timestamp.
struct PriceData {
    std::string symbol;                                          // Stock symbol (e.g., "AAPL", "BTC")
    double price;                                                // Current price
    double change;                                               // Price change from previous tick
    std::chrono::high_resolution_clock::time_point timestamp;    // High-precision timestamp for latency measurement
    
    PriceData() : price(0.0), change(0.0) {}
    
    PriceData(const std::string& sym, double p, double c) 
        : symbol(sym), price(p), change(c), 
          timestamp(std::chrono::high_resolution_clock::now()) {}
};

// Captures basic timing info for one operation.
struct PerformanceMetrics {
    std::chrono::high_resolution_clock::time_point generation_time;  // When price was generated
    std::chrono::high_resolution_clock::time_point processing_time;  // When indicator was calculated
    std::string operation;                                            // Description of operation
    double latency_microseconds;                                      // Calculated latency
    
    PerformanceMetrics() : latency_microseconds(0.0) {}
};

#endif // PRICE_DATA_H
