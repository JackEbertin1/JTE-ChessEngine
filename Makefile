# Compiler and flags
# Try to use Homebrew LLVM if available, otherwise fall back to system clang++
LLVM_PREFIX := $(shell brew --prefix llvm 2>/dev/null)
ifneq ($(LLVM_PREFIX),)
  CXX = $(LLVM_PREFIX)/bin/clang++
  CXXFLAGS = -Wall -std=c++20 -I$(PWD)/include -I$(LLVM_PREFIX)/include/c++/v1
  LDFLAGS = -L$(LLVM_PREFIX)/lib/c++ -L$(LLVM_PREFIX)/lib/unwind -Wl,-rpath,$(LLVM_PREFIX)/lib/c++ -Wl,-rpath,$(LLVM_PREFIX)/lib -lunwind -stdlib=libc++
else
  CXX = clang++
  SDKROOT := $(shell xcrun --show-sdk-path 2>/dev/null)
  CXXFLAGS = -Wall -std=c++20 -I$(PWD)/include
  ifneq ($(SDKROOT),)
    CXXFLAGS += -isysroot $(SDKROOT) -stdlib=libc++
  endif
endif

# Target executable
TARGET = bin/chessEngine

# Source directories
SRC_DIR = src
OBJ_DIR = obj

# Find all source files recursively
SRCS = $(shell find $(SRC_DIR) -name "*.cpp")

# Generate object file names
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Default target
all: $(TARGET)

# Create directories if they don't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Link object files to create executable
$(TARGET): $(OBJS) | bin
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# Create bin directory
bin:
	mkdir -p bin

# Compile .cpp files to .o object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJ_DIR) bin

# Debug target with additional flags
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

# Release target with optimization
release: CXXFLAGS += -O3 -DNDEBUG
release: $(TARGET)

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build the chess engine (default)"
	@echo "  debug    - Build with debug symbols"
	@echo "  release  - Build with optimization"
	@echo "  clean    - Remove build files"
	@echo "  help     - Show this help message"

.PHONY: all clean debug release help