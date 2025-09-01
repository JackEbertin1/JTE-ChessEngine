cmake_minimum_required(VERSION 3.16)
project(ChessEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(include)

# Find source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(chessEngine ${SOURCES})

# Set target properties
set_target_properties(chessEngine PROPERTIES
    OUTPUT_NAME "chessEngine"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Compiler-specific flags
target_compile_features(chessEngine PRIVATE cxx_std_20)

# Optional: Enable testing
option(BUILD_TESTS "Build test suite" OFF)
if(BUILD_TESTS)
    enable_testing()
    # Add test targets here if you have tests
endif()

# Optional: Enable sanitizers
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
if(ENABLE_SANITIZERS)
    target_compile_options(chessEngine PRIVATE -fsanitize=address,undefined)
    target_link_options(chessEngine PRIVATE -fsanitize=address,undefined)
endif()

# Optional: Enable profiling
option(ENABLE_PROFILING "Enable profiling" OFF)
if(ENABLE_PROFILING)
    target_compile_options(chessEngine PRIVATE -pg)
    target_link_options(chessEngine PRIVATE -pg)
endif()

# Install rules
install(TARGETS chessEngine
    RUNTIME DESTINATION bin
)

# Print configuration summary
message(STATUS "Chess Engine Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  Enable Sanitizers: ${ENABLE_SANITIZERS}")
message(STATUS "  Enable Profiling: ${ENABLE_PROFILING}")
