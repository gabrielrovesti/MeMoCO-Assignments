# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -DIL_STD

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# CPLEX directories - for Linux lab environment
CPLEX_DIR = /opt/ibm/ILOG/CPLEX_Studio2211
CPLEX_INCDIR = $(CPLEX_DIR)/cplex/include
CONCERT_INCDIR = $(CPLEX_DIR)/concert/include
SYSTEM = x86-64_linux
LIBFORMAT = static_pic
CPLEX_LIBDIR = $(CPLEX_DIR)/cplex/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERT_LIBDIR = $(CPLEX_DIR)/concert/lib/$(SYSTEM)/$(LIBFORMAT)

# Include paths
INCLUDES = -I$(INC_DIR) -I$(CPLEX_INCDIR) -I$(CONCERT_INCDIR)

# Libraries
LDFLAGS = -L$(CPLEX_LIBDIR) -L$(CONCERT_LIBDIR)
LDLIBS = -lilocplex -lcplex -lconcert -lm -lpthread -ldl

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/ilolpex1

# Create necessary directories
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Main target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

# Compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean