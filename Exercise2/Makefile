# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -DIL_STD

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
DATA_DIR = data
RESULTS_DIR = results
VIS_DIR = visualizations

# C++17 filesystem support
LDFLAGS = -lstdc++fs

# Include paths
INCLUDES = -I$(INC_DIR)

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/ilolpex1

# Create all necessary directories
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DATA_DIR) $(RESULTS_DIR) $(VIS_DIR))

# Main target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(DATA_DIR)
	@mkdir -p $(RESULTS_DIR)
	@mkdir -p $(VIS_DIR)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean built files but preserve data and results
clean:
	rm -rf $(BUILD_DIR)

# Clean everything including data and results
distclean: clean
	rm -rf $(DATA_DIR)/* $(RESULTS_DIR)/* $(VIS_DIR)/*

# Print debugging information
debug:
	@echo "Compiler: $(CXX)"
	@echo "Compiler flags: $(CXXFLAGS)"
	@echo "Source files: $(SRCS)"
	@echo "Object files: $(OBJS)"
	@echo "Include paths: $(INCLUDES)"
	@echo "Libraries: $(LDFLAGS)"

.PHONY: all clean distclean debug directories