# 📚 Project File Index

## Quick Navigation

### 🚀 Getting Started
1. **[PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)** - Start here! Complete overview and status
2. **[QuickStart.md](QuickStart.md)** - Build and run in 2 minutes
3. **[README.md](README.md)** - Comprehensive technical documentation

### 📖 Documentation
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - Visual diagrams and system design
- **[PerformanceReportOutline.md](PerformanceReportOutline.md)** - Academic report template (3-5 pages)

### 💻 Source Code

#### Core Implementation (Header Files)
- **[PriceData.h](PriceData.h)** (1.6 KB) - Data structures for price ticks and metrics
- **[SharedBuffer.h](SharedBuffer.h)** (6.8 KB) - Thread-safe circular buffer with mutex & CV
- **[PerformanceMonitor.h](PerformanceMonitor.h)** (9.8 KB) - Performance tracking system

#### Thread Implementations
- **[PriceGenerator.h](PriceGenerator.h)** (6.5 KB) - Producer thread (generates prices)
- **[DisplayThread.h](DisplayThread.h)** (4.7 KB) - Consumer thread (displays prices)
- **[SMACalculator.h](SMACalculator.h)** (6.6 KB) - Consumer thread (calculates moving average)
- **[VolatilityCalculator.h](VolatilityCalculator.h)** (8.1 KB) - Consumer thread (calculates volatility)

#### Main Application
- **[main.cpp](main.cpp)** (8.5 KB) - Entry point, thread orchestration, shutdown coordination

### 🛠️ Build System
- **[CMakeLists.txt](CMakeLists.txt)** - CMake build configuration
- **[.gitignore](.gitignore)** - Version control ignore rules

### 📦 Binary
- **stock_simulator.exe** (149 KB) - Compiled executable (ready to run!)

---

## File Purpose Summary

| File | Type | Purpose | Lines | Key Concepts |
|------|------|---------|-------|--------------|
| PriceData.h | Header | Data structures | ~40 | Structs, timestamps |
| SharedBuffer.h | Header | Thread-safe buffer | ~200 | Mutex, condition variables, circular buffer |
| PriceGenerator.h | Header | Producer thread | ~150 | Thread creation, random generation |
| DisplayThread.h | Header | Consumer thread | ~110 | Console I/O, data visualization |
| SMACalculator.h | Header | Consumer thread | ~150 | Moving average, latency tracking |
| VolatilityCalculator.h | Header | Consumer thread | ~180 | Standard deviation, risk metrics |
| PerformanceMonitor.h | Header | Metrics system | ~220 | High-resolution timing, statistics |
| main.cpp | Source | Application core | ~180 | Thread management, shutdown |
| README.md | Doc | Technical guide | ~450 | Architecture, compilation, usage |
| ARCHITECTURE.md | Doc | Visual diagrams | ~350 | System design, data flow |
| PerformanceReportOutline.md | Doc | Report template | ~400 | Academic structure |
| PROJECT_SUMMARY.md | Doc | Project overview | ~200 | Status, deliverables |
| QuickStart.md | Doc | Fast setup | ~50 | Build & run |
| CMakeLists.txt | Build | CMake config | ~40 | Cross-platform build |
| .gitignore | Config | Git ignore | ~30 | Build artifacts |

**Total Code**: ~1,200 lines of well-commented C++  
**Total Documentation**: ~1,500 lines of markdown  
**Total Project**: ~2,700 lines

---

## Usage Workflow

### For Compilation
```
QuickStart.md → Compile → stock_simulator.exe
```

### For Understanding
```
PROJECT_SUMMARY.md → README.md → ARCHITECTURE.md → Source Code
```

### For Report Writing
```
PerformanceReportOutline.md → Run Tests → Fill Results → Submit
```

---

## Key Files by Task

### 🎯 Want to run the program?
→ **QuickStart.md** or **README.md** (Compilation section)

### 📝 Want to write the report?
→ **PerformanceReportOutline.md**

### 🔍 Want to understand the code?
→ **ARCHITECTURE.md** then individual .h files

### 🐛 Want to debug/modify?
→ Start with **SharedBuffer.h** (core synchronization)

### 📊 Want to analyze performance?
→ **PerformanceMonitor.h** + run with different durations

---

## Code Reading Order (Recommended)

For understanding the system architecture:

1. **[PriceData.h](PriceData.h)** - Understand data structures first
2. **[SharedBuffer.h](SharedBuffer.h)** - Learn synchronization mechanisms
3. **[PriceGenerator.h](PriceGenerator.h)** - See producer pattern
4. **[SMACalculator.h](SMACalculator.h)** - See consumer pattern
5. **[main.cpp](main.cpp)** - See everything working together

---

## Documentation Reading Order

For comprehensive understanding:

1. **[PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)** - 5 minutes - Overview
2. **[QuickStart.md](QuickStart.md)** - 2 minutes - Get it running
3. **[README.md](README.md)** - 15 minutes - Deep technical dive
4. **[ARCHITECTURE.md](ARCHITECTURE.md)** - 10 minutes - Visual understanding
5. **[PerformanceReportOutline.md](PerformanceReportOutline.md)** - Reference - When writing report

**Total reading time**: ~30-40 minutes for complete understanding

---

## Quick Reference

### Compilation Command
```bash
g++ -std=c++17 -pthread -O2 -o stock_simulator.exe main.cpp -Wall -Wextra
```

### Run Command
```bash
.\stock_simulator.exe 30
```

### Key Classes
- `SharedBuffer` - Thread-safe data sharing
- `PriceGenerator` - Producer thread
- `DisplayThread`, `SMACalculator`, `VolatilityCalculator` - Consumer threads
- `PerformanceMonitor` - Metrics collection

### Synchronization Primitives
- `std::mutex` - Mutual exclusion
- `std::condition_variable` - Producer-consumer coordination
- `std::atomic<bool>` - Thread-safe flags
- `std::unique_lock` - RAII locking

---

## File Dependency Graph

```
main.cpp
├── SharedBuffer.h
│   └── PriceData.h
├── PriceGenerator.h
│   ├── SharedBuffer.h
│   ├── PriceData.h
│   └── PerformanceMonitor.h
├── DisplayThread.h
│   ├── SharedBuffer.h
│   └── PriceData.h
├── SMACalculator.h
│   ├── SharedBuffer.h
│   ├── PriceData.h
│   └── PerformanceMonitor.h
└── VolatilityCalculator.h
    ├── SharedBuffer.h
    ├── PriceData.h
    └── PerformanceMonitor.h
```

**Note**: All headers are self-contained and include-guarded.

---

## 🎓 Educational Value by File

| File | Concepts Taught |
|------|----------------|
| SharedBuffer.h | Mutex, condition variables, RAII, critical sections |
| PriceGenerator.h | Thread creation, atomic operations, random generation |
| SMACalculator.h | Consumer pattern, mathematical computation |
| VolatilityCalculator.h | Statistical analysis, independent consumers |
| main.cpp | Thread lifecycle, graceful shutdown, coordination |
| PerformanceMonitor.h | High-resolution timing, performance analysis |

---

**Last Updated**: December 26, 2025  
**Project Status**: ✅ Complete and Tested  
**Total Files**: 16 (7 headers, 1 source, 5 docs, 3 config)
