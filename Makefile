# Define variables for the compiler and flags
CXX = g++
# Use -O0 or -Og during development for faster compilation times than -O2/-O3
CXXFLAGS = -std=c++20 -Wall -Wextra -g -O0
LDFLAGS = -lraylib

# Define the target executable name
TARGET = game

# List all source files (.cc)
SRCS =  src/main.cc \
		src/game.cc \
		src/entity.cc \
		src/enemy.cc \
		src/enemies.cc \
		src/entities.cc

# Automatically generate a list of object files (.o) in a build directory
# This assumes you have a 'build' directory ready (mkdir build)
OBJS = $(patsubst %.cc, build/%.o, $(SRCS))

# --- Rules ---

# The default rule: build the target executable from object files
$(TARGET): $(OBJS)
	@echo "Linking..."
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Rule to compile a single .cc file into a .o file
# This rule utilizes ccache automatically because we assume 'g++' resolves to 'ccache g++' via PATH
build/%.o: %.cc
	@echo "Compiling $<..."
	@mkdir -p $(dir $@) # Ensure the directory exists
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule: remove generated files
clean:
	@echo "Cleaning up..."
	rm -rf build/ $(TARGET)

.PHONY: all clean
