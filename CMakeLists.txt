cmake_minimum_required(VERSION 3.15)
project(DualColonyACO 
        VERSION 1.0.0
        DESCRIPTION "Dual-Colony Ant Colony Optimization for Dynamic TSP"
        LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Define the include directory
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files for the main executable
set(SOURCES
    src/main.cpp
    src/TrafficGraph.cpp
    src/Ant.cpp
    src/ColonyManager.cpp
)

# Create the executable
add_executable(DualColonyACO ${SOURCES})

# Set compiler flags for optimization and warnings
if(MSVC)
    # Microsoft Visual C++
    target_compile_options(DualColonyACO PRIVATE /W4 /O2)
else()
    # GCC/Clang
    target_compile_options(DualColonyACO PRIVATE -Wall -Wextra -Wpedantic -O2)
endif()

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(DualColonyACO PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(DualColonyACO PRIVATE DUALCOLONY_OPENMP=1)
    message(STATUS "OpenMP enabled for run-level parallelism.")
else()
    message(STATUS "OpenMP not found; building with sequential execution.")
endif()

# Print build information
message(STATUS "DualColonyACO configured for C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Executable output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DualColonyACO")
