CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g -ltbb
LDFLAGS = -lraylib
TARGET = game

# 1. Detect all .cc files in the src/ directory
ALL_SRCS = $(wildcard src/*.cc)

# 2. Extract main.cc and everything else separately
MAIN_SRC = src/main.cc
OTHER_SRCS = $(filter-out $(MAIN_SRC), $(ALL_SRCS))

# 3. Combine them ensuring main.cc is first
SRCS = $(MAIN_SRC) $(OTHER_SRCS)

# Automatically generate object paths in the build directory
OBJS = $(patsubst %.cc, build/%.o, $(SRCS))

# --- Rules ---

$(TARGET): $(OBJS)
	@echo "Linking..."
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

build/%.o: %.cc
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
run: $(TARGET)
	@echo "Running $(TARGET) with game audio priority..."
	@export LD_LIBRARY_PATH=$${LD_LIBRARY_PATH}:. && \
	PULSE_PROP="media.role=game" ./$(TARGET)

debug: $(TARGET)
	@echo "Launching GDB..."
	@export LD_LIBRARY_PATH=$${LD_LIBRARY_PATH}:. && gdb -ex run ./$(TARGET)

# Optional: Add a memory check target using Valgrind
memcheck: $(TARGET)
	@echo "Checking for memory leaks..."
	@export LD_LIBRARY_PATH=$${LD_LIBRARY_PATH}:. && valgrind --leak-check=full ./$(TARGET)

clean:
	@echo "Cleaning up..."
	rm -rf build/ $(TARGET)

.PHONY: all clean run debug memcheck


