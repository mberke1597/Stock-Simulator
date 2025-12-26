# Quick Start Guide

## Build & Run Commands

### Windows (PowerShell)

```powershell
# Using MinGW g++
g++ -std=c++17 -pthread -O2 -o stock_simulator.exe main.cpp -Wall -Wextra

# Run (30 seconds default)
.\stock_simulator.exe

# Run with custom duration (60 seconds)
.\stock_simulator.exe 60
```

### Linux/macOS

```bash
# Using g++
g++ -std=c++17 -pthread -O2 -o stock_simulator main.cpp -Wall -Wextra

# Run
./stock_simulator

# Run with custom duration
./stock_simulator 60
```

### Using CMake (Optional)

Create `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.10)
project(StockSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(stock_simulator main.cpp)

if(UNIX)
    target_link_libraries(stock_simulator pthread)
endif()
```

Build:
```bash
mkdir build
cd build
cmake ..
cmake --build .
./stock_simulator
```

## Controls

- **Ctrl+C**: Stop simulation early and view performance report
- Program automatically stops after specified duration

## Troubleshooting

### Linker Error: undefined reference to pthread_create
**Solution**: Add `-pthread` flag to compilation command

### Windows: 'g++' is not recognized
**Solution**: Install MinGW or use MSVC (Visual Studio)

### Performance issues
**Solution**: Compile with `-O2` optimization flag
