# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
DATA_DIR = data
RESULTS_DIR = results
VIS_DIR = visualizations

# Include paths
INCLUDES = -I$(INC_DIR)

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/ilolpex1

# Create all necessary directories
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DATA_DIR) $(RESULTS_DIR) $(VIS_DIR))

# Main target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

# Compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean built files but preserve data and results
clean:
	rm -rf $(BUILD_DIR)

# Clean everything including data and results
distclean: clean
	rm -rf $(DATA_DIR)/* $(RESULTS_DIR)/* $(VIS_DIR)/*

.PHONY: all clean distclean