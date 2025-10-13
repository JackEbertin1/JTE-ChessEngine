CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Iinclude
SRCDIR := src
OBJDIR := build
TARGET := chessGame

SOURCES := $(shell find $(SRCDIR) -name '*.cpp')
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
    $(CXX) $(CXXFLAGS) -o $@ $^

# Compile .cpp to .o
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
    @mkdir -p $(dir $@)
    $(CXX) $(CXXFLAGS) -c $< -o $@

clean:
    rm -rf $(OBJDIR) $(TARGET)