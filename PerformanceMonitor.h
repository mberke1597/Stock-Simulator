#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include "PriceData.h"
#include <chrono>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <algorithm>

// Collects simple latency and throughput stats for the simulator.
class PerformanceMonitor {
private:
    // Latency measurements per symbol and operation
    struct LatencyRecord {
        std::chrono::high_resolution_clock::time_point generation_time;
        std::chrono::high_resolution_clock::time_point processing_time;
        std::string operation;  // "SMA", "Volatility", etc.
        double latency_microseconds;
    };
    
    std::map<std::string, std::vector<LatencyRecord>> latency_records_;  // symbol -> records
    std::map<std::string, std::chrono::high_resolution_clock::time_point> generation_times_;  // symbol -> last gen time
    
    mutable std::mutex mutex_;  // Protects all shared data
    
    // System start time for uptime calculation
    std::chrono::high_resolution_clock::time_point start_time_;
    
    // Statistics
    size_t total_generations_;
    size_t total_calculations_;

public:
    PerformanceMonitor() 
        : start_time_(std::chrono::high_resolution_clock::now()),
          total_generations_(0), total_calculations_(0) {}
    
    void recordGeneration(const std::string& symbol, 
                         const std::chrono::high_resolution_clock::time_point& timestamp) {
        std::unique_lock<std::mutex> lock(mutex_);
        generation_times_[symbol] = timestamp;
        ++total_generations_;
    }
    
    void recordProcessing(const std::string& symbol,
                         const std::string& operation,
                         const std::chrono::high_resolution_clock::time_point& generation_time,
                         const std::chrono::high_resolution_clock::time_point& processing_time) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        LatencyRecord record;
        record.generation_time = generation_time;
        record.processing_time = processing_time;
        record.operation = operation;
        
        // Calculate latency in microseconds
        record.latency_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
            processing_time - generation_time).count();
        
        latency_records_[symbol].push_back(record);
        ++total_calculations_;
    }
    
    void getLatencyStats(const std::string& symbol,
                        const std::string& operation,
                        double& min_latency,
                        double& max_latency,
                        double& avg_latency,
                        size_t& sample_count) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        auto it = latency_records_.find(symbol);
        if (it == latency_records_.end()) {
            min_latency = max_latency = avg_latency = 0.0;
            sample_count = 0;
            return;
        }
        
        // Filter records by operation
        std::vector<double> latencies;
        for (const auto& record : it->second) {
            if (record.operation == operation) {
                latencies.push_back(record.latency_microseconds);
            }
        }
        
        if (latencies.empty()) {
            min_latency = max_latency = avg_latency = 0.0;
            sample_count = 0;
            return;
        }
        
        // Calculate statistics
        min_latency = *std::min_element(latencies.begin(), latencies.end());
        max_latency = *std::max_element(latencies.begin(), latencies.end());
        avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
        sample_count = latencies.size();
    }
    
    void printReport() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        auto now = std::chrono::high_resolution_clock::now();
        auto uptime_seconds = std::chrono::duration_cast<std::chrono::seconds>(
            now - start_time_).count();
        
        std::cout << "\n\n";
        std::cout << "====================================================\n";
        std::cout << "           PERFORMANCE MONITORING REPORT            \n";
        std::cout << "====================================================\n\n";
        
        std::cout << "System Uptime: " << uptime_seconds << " seconds\n";
        std::cout << "Total Price Generations: " << total_generations_ << "\n";
        std::cout << "Total Indicator Calculations: " << total_calculations_ << "\n";
        
        if (uptime_seconds > 0) {
            double gen_per_sec = static_cast<double>(total_generations_) / uptime_seconds;
            double calc_per_sec = static_cast<double>(total_calculations_) / uptime_seconds;
            std::cout << "Generation Rate: " << std::fixed << std::setprecision(2) 
                      << gen_per_sec << " ops/sec\n";
            std::cout << "Calculation Rate: " << calc_per_sec << " ops/sec\n";
        }
        
        std::cout << "\n--- Latency Statistics (microseconds) ---\n\n";
        std::cout << std::setw(10) << "Symbol" 
                  << std::setw(15) << "Operation" 
                  << std::setw(12) << "Samples"
              << std::setw(12) << "Min (us)"
              << std::setw(12) << "Max (us)"
              << std::setw(12) << "Avg (us)" << "\n";
        std::cout << std::string(73, '-') << "\n";
        
        for (const auto& symbol_pair : latency_records_) {
            const std::string& symbol = symbol_pair.first;
            
            // Get unique operations for this symbol
            std::vector<std::string> operations;
            for (const auto& record : symbol_pair.second) {
                if (std::find(operations.begin(), operations.end(), record.operation) == operations.end()) {
                    operations.push_back(record.operation);
                }
            }
            
            // Print stats for each operation
            for (const auto& op : operations) {
                std::vector<double> latencies;
                for (const auto& record : symbol_pair.second) {
                    if (record.operation == op) {
                        latencies.push_back(record.latency_microseconds);
                    }
                }
                
                if (!latencies.empty()) {
                    double min_lat = *std::min_element(latencies.begin(), latencies.end());
                    double max_lat = *std::max_element(latencies.begin(), latencies.end());
                    double avg_lat = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
                    
                    std::cout << std::setw(10) << symbol
                              << std::setw(15) << op
                              << std::setw(12) << latencies.size()
                              << std::setw(12) << std::fixed << std::setprecision(2) << min_lat
                              << std::setw(12) << max_lat
                              << std::setw(12) << avg_lat << "\n";
                }
            }
        }
        
        std::cout << "\n====================================================\n\n";
    }
    
    void getSystemStats(size_t& generations, size_t& calculations, double& uptime_seconds) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        auto now = std::chrono::high_resolution_clock::now();
        uptime_seconds = std::chrono::duration_cast<std::chrono::seconds>(
            now - start_time_).count();
        
        generations = total_generations_;
        calculations = total_calculations_;
    }
};

#endif // PERFORMANCE_MONITOR_H
