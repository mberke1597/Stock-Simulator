cmake_minimum_required(VERSION 3.10)
project(StockPriceSimulator CXX)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Optimization flags for Release builds
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Add executable
add_executable(stock_simulator main.cpp)

# Link pthread library on Unix-like systems
if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(stock_simulator Threads::Threads)
endif()

# On Windows with MinGW, also link pthread
if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_link_libraries(stock_simulator pthread)
endif()

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
