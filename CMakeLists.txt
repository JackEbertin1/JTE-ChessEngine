cmake_minimum_required(VERSION 3.23)
project(chessGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Qt path for Homebrew
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@6")

# ==================================================
# Chess Engine Library (shared between CLI and GUI)
# ==================================================
file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_SOURCE_DIR}/src/chess/pieces/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/chess/representation/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/chess/search/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/chess/evaluation/*.cpp"
)

add_library(ChessEngine STATIC ${ENGINE_SOURCES})
target_include_directories(ChessEngine PUBLIC "${CMAKE_SOURCE_DIR}/include")

# compiler flags and threads for engine
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(ChessEngine PRIVATE -Wall -Wextra -O2)
endif()

find_package(Threads REQUIRED)
target_link_libraries(ChessEngine PUBLIC Threads::Threads)

# ==================================================
# CLI Executable (command-line version)
# ==================================================
add_executable(chessGame_CLI "${CMAKE_SOURCE_DIR}/src/main.cpp")
target_link_libraries(chessGame_CLI PRIVATE ChessEngine)

# ==================================================
# GUI Executable (Qt version)
# ==================================================
find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB GUI_SOURCES "${CMAKE_SOURCE_DIR}/src/gui/*.cpp")
file(GLOB GUI_HEADERS "${CMAKE_SOURCE_DIR}/include/gui/*.h")

add_executable(chessGame_GUI ${GUI_SOURCES} ${GUI_HEADERS})
target_link_libraries(chessGame_GUI PRIVATE
    ChessEngine
    Qt6::Widgets
)
target_include_directories(chessGame_GUI PRIVATE "${CMAKE_SOURCE_DIR}/include")