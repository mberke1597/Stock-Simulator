# Real-Time Stock Price Simulator

## Overview

A sophisticated multithreaded C++ application designed to demonstrate core **Operating Systems concepts** including multithreading, synchronization primitives, producer-consumer patterns, and performance measurement. This academic project simulates real-time stock price fluctuations and calculates technical indicators using concurrent processing.

## 🎯 Educational Objectives

This project demonstrates:
- ✅ **Multithreading** with `std::thread`
- ✅ **Synchronization** with `std::mutex` and `std::condition_variable`
- ✅ **Producer-Consumer Pattern** implementation
- ✅ **Thread-Safe Shared Memory** (Circular Buffer)
- ✅ **Deadlock-Free Design** principles
- ✅ **Race Condition Prevention** strategies
- ✅ **High-Resolution Performance Measurement** with `std::chrono`

## 🏗️ System Architecture

### Thread Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Main Thread                             │
│              (Coordination & Shutdown)                       │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   │ Spawns & Manages
                   │
    ┌──────────────┼──────────────┬────────────────┐
    │              │              │                │
    ▼              ▼              ▼                ▼
┌────────┐   ┌──────────┐   ┌─────────┐   ┌──────────────┐
│Producer│   │Consumer  │   │Consumer │   │   Consumer   │
│ Thread │   │ (Display)│   │  (SMA)  │   │(Volatility)  │
│        │   │          │   │         │   │              │
│  Gen   │──▶│  Shows   │   │  Calc   │   │    Calc      │
│ Prices │   │  Prices  │   │  Moving │   │  Volatility  │
│        │   │          │   │  Average│   │              │
└────┬───┘   └────▲─────┘   └────▲────┘   └──────▲───────┘
     │            │              │                │
     │            │              │                │
     └────────────┴──────────────┴────────────────┘
                         │
                         ▼
              ┌──────────────────────┐
              │   Shared Buffer      │
              │  (Thread-Safe)       │
              │  ┌─────────────┐     │
              │  │ Mutex Lock  │     │
              │  │ Cond Vars   │     │
              │  │ Circ Buffer │     │
              │  └─────────────┘     │
              └──────────────────────┘
```

### Component Breakdown

#### **Thread 1: Price Generator (Producer)**
- **File**: `PriceGenerator.h`
- **Responsibility**: Generates random price fluctuations for multiple stock symbols
- **Update Rate**: 100ms intervals
- **Synchronization**: Pushes data to shared buffer with mutex protection
- **Symbols**: AAPL, GOOGL, MSFT, AMZN, BTC

#### **Thread 2: Display (Consumer)**
- **File**: `DisplayThread.h`
- **Responsibility**: Real-time console visualization
- **Update Rate**: 500ms intervals
- **Synchronization**: Uses condition variable to wait for new data

#### **Thread 3: SMA Calculator (Consumer)**
- **File**: `SMACalculator.h`
- **Responsibility**: Calculates Simple Moving Average
- **Window Size**: 20 periods
- **Update Rate**: 1000ms intervals
- **Synchronization**: Reads price history with mutex protection

#### **Thread 4: Volatility Calculator (Consumer)**
- **File**: `VolatilityCalculator.h`
- **Responsibility**: Calculates volatility (standard deviation of returns)
- **Window Size**: 20 periods
- **Update Rate**: 1500ms intervals
- **Synchronization**: Independent consumer, no inter-thread dependencies

## 🔒 Synchronization Strategy

### Mutex Protection (`std::mutex`)

**Purpose**: Prevents race conditions when accessing shared data

**Usage in `SharedBuffer`**:
```cpp
void push(const PriceData& data) {
    {
        std::unique_lock<std::mutex> lock(mutex_);  // CRITICAL SECTION START
        // Modify shared price_history_
        price_history_[data.symbol].push_back(data);
        ++total_writes_;
    }  // CRITICAL SECTION END (lock automatically released)
    
    cv_data_ready_.notify_all();  // Signal consumers OUTSIDE lock
}
```

**Key Design Decisions**:
- Single mutex per shared buffer (no nested locks → no deadlock)
- RAII locking with `std::unique_lock` (automatic release)
- Notifications sent OUTSIDE critical section (reduces lock contention)

### Condition Variables (`std::condition_variable`)

**Purpose**: Efficient producer-consumer synchronization without busy-waiting

**Consumer Wait Pattern**:
```cpp
bool waitForData(int timeout_ms) {
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_data_ready_.wait_for(lock, 
                                    std::chrono::milliseconds(timeout_ms),
                                    [this] { return shutdown_ || total_writes_ > 0; });
}
```

**Benefits**:
- Avoids CPU-wasting busy loops
- Consumers sleep until producer signals new data
- Bounded wait times prevent indefinite blocking

### Deadlock Prevention

**Strategy**: Consistent lock ordering and single-lock design

**Guarantees**:
1. ✅ **Single Mutex**: Only one mutex per SharedBuffer
2. ✅ **No Nested Locks**: Never acquire multiple locks
3. ✅ **Timeout-Based Waits**: All waits have maximum duration
4. ✅ **Notify Outside Lock**: Signal condition variables after releasing mutex

## 📊 Performance Measurement

### High-Resolution Timing (`std::chrono`)

**Latency Tracking**:
```cpp
// Producer records generation time
auto generation_time = std::chrono::high_resolution_clock::now();
data.timestamp = generation_time;

// Consumer calculates processing latency
auto processing_time = std::chrono::high_resolution_clock::now();
auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
    processing_time - generation_time).count();
```

**Metrics Collected**:
- ⏱️ Generation-to-calculation latency (microseconds)
- 📈 Throughput (operations per second)
- 📊 Min/Max/Average latency per operation
- 🔢 Total read/write operations
- ⏰ System uptime

## 🛠️ Compilation & Execution

### Prerequisites

- **Compiler**: g++ 7.0+ or clang 5.0+ (C++17 support)
- **OS**: Windows, Linux, or macOS
- **Libraries**: Standard C++ library (no external dependencies)

### Compile Commands

#### **Linux/macOS (g++)**:
```bash
g++ -std=c++17 -pthread -O2 -o stock_simulator main.cpp -Wall -Wextra
```

#### **Linux/macOS (clang)**:
```bash
clang++ -std=c++17 -pthread -O2 -o stock_simulator main.cpp -Wall -Wextra
```

#### **Windows (MinGW)**:
```bash
g++ -std=c++17 -pthread -O2 -o stock_simulator.exe main.cpp -Wall -Wextra
```

#### **Windows (MSVC)**:
```bash
cl /std:c++17 /EHsc /O2 main.cpp /Fe:stock_simulator.exe
```

### Run the Simulator

**Default (30 seconds)**:
```bash
./stock_simulator
```

**Custom runtime** (e.g., 60 seconds):
```bash
./stock_simulator 60
```

**Graceful shutdown**:
- Press `Ctrl+C` to stop early and view performance report

### Expected Output

```
========================================================
    REAL-TIME STOCK PRICE SIMULATOR (Multithreaded)    
========================================================

[Main] Starting all threads...
[PriceGenerator] Started producer thread (ID: 12345)
[DisplayThread] Started display thread (ID: 12346)
[SMACalculator] Started SMA calculator thread (ID: 12347)
[VolatilityCalculator] Started volatility calculator thread (ID: 12348)

========== REAL-TIME STOCK PRICE MONITOR ==========

AAPL: $ 234.56 ↑ +0.45 | GOOGL: $ 142.78 ↓ -0.23 | ...

[SMACalculator] AAPL - Price: $234.56 | SMA(20): $233.12 | Deviation: +0.62% | Calc time: 145 μs
[VolatilityCalculator] AAPL - Volatility: 24.3% (annualized) | Level: MODERATE | Calc time: 183 μs

====================================================
           PERFORMANCE MONITORING REPORT            
====================================================

System Uptime: 30 seconds
Total Price Generations: 1500
Total Indicator Calculations: 450
Generation Rate: 50.00 ops/sec
Calculation Rate: 15.00 ops/sec

--- Latency Statistics (microseconds) ---

Symbol     Operation      Samples     Min (μs)    Max (μs)    Avg (μs)
-------------------------------------------------------------------------
AAPL       SMA               30       120.45      450.23      245.67
AAPL       Volatility        20       150.34      520.12      298.45
...
====================================================
```

## 📁 File Structure

```
OSProject/
│
├── main.cpp                    # Application entry point & thread management
├── PriceData.h                 # Core data structures
├── SharedBuffer.h              # Thread-safe circular buffer
├── PriceGenerator.h            # Producer thread implementation
├── DisplayThread.h             # Consumer thread (UI)
├── SMACalculator.h             # Consumer thread (SMA indicator)
├── VolatilityCalculator.h      # Consumer thread (Volatility indicator)
├── PerformanceMonitor.h        # Performance tracking system
├── README.md                   # This file
└── PerformanceReportOutline.md # Academic report template
```

## 🧪 Testing & Validation

### Thread Safety Verification

**Tools**:
- **Valgrind** (Linux): Detect race conditions
  ```bash
  valgrind --tool=helgrind ./stock_simulator 10
  ```

- **Thread Sanitizer** (Clang):
  ```bash
  clang++ -std=c++17 -pthread -fsanitize=thread -g main.cpp -o stock_simulator
  ./stock_simulator 10
  ```

### Performance Profiling

**CPU Usage Monitoring** (Linux):
```bash
top -H -p $(pgrep stock_simulator)
```

**Memory Usage** (Linux):
```bash
valgrind --tool=massif ./stock_simulator 30
```

## 🔬 Key Concepts Demonstrated

### 1. Race Condition Prevention
- All shared data access protected by mutex
- RAII locking ensures no forgotten unlocks
- Atomic operations for simple flags

### 2. Deadlock Prevention
- Single-lock design (no circular wait)
- Consistent lock ordering
- Timeouts on all blocking operations

### 3. Producer-Consumer Pattern
- Producer generates data independently
- Multiple consumers process data concurrently
- Condition variables coordinate efficiently

### 4. Resource Management
- RAII for automatic resource cleanup
- Graceful shutdown mechanism
- Thread join guarantees completion

## 📝 Academic Report Support

See [`PerformanceReportOutline.md`](PerformanceReportOutline.md) for a structured template to write your 3-5 page performance analysis report.

**Report Sections**:
1. System Architecture & Design
2. Synchronization Strategy & Implementation
3. Performance Analysis & Results
4. Conclusion & Future Work

## 🚀 Extensions & Improvements

**Possible Enhancements**:
- [ ] Add more technical indicators (RSI, MACD, Bollinger Bands)
- [ ] Implement priority-based scheduling
- [ ] Add thread pool for scalability
- [ ] Export metrics to CSV for analysis
- [ ] Implement lock-free data structures
- [ ] Add configurable thread priorities
- [ ] Implement custom allocator for performance

## 📚 References

- C++ Concurrency in Action (Anthony Williams)
- Modern Operating Systems (Tanenbaum & Bos)
- The Art of Multiprocessor Programming (Herlihy & Shavit)
- C++ Standard Library Documentation: https://en.cppreference.com

## 📄 License

This is an academic project for educational purposes.

## 👤 Author

Academic Project demonstrating Operating Systems concepts
Date: December 2025
Language: C++17/20

---

**Note**: This simulator uses random price generation and is for educational purposes only. Do not use for actual trading decisions.
