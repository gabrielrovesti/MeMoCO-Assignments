# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2

# CPLEX directories - MODIFY THESE PATHS
CPLEX_DIR = /opt/ibm/ILOG/CPLEX_Studio2211/cplex
CONCERT_DIR = /opt/ibm/ILOG/CPLEX_Studio2211/concert

# Include directories
INCLUDES = -I$(CPLEX_DIR)/include -I$(CONCERT_DIR)/include

# Library directories and libraries
LDFLAGS = -L$(CPLEX_DIR)/lib/x86-64_linux/static_pic -L$(CONCERT_DIR)/lib/x86-64_linux/static_pic
LDLIBS = -lilocplex -lcplex -lpthread -ldl

# Source files
SRCS = src/main.cpp src/model.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = tsp_solver

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

# Compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(TARGET)

# Run
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run