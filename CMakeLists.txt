cmake_minimum_required(VERSION 3.10)
project(chessGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# Collect sources
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# Create executable named chessGame
add_executable(chessGame ${SOURCES})

# Compile options (GCC/Clang)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(chessGame PRIVATE -Wall -Wextra -O2)
endif()

target_include_directories(chessGame PRIVATE ${PROJECT_SOURCE_DIR}/include)