CC = g++
CPPFLAGS = -g -Wall -O2
LDFLAGS =

# Object files
OBJ = main.o TSPSolver.o TSPSolution.o TSP.o ParameterCalibration.o

# Build rules
%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $^ -o $@

# Main target
main: $(OBJ)
	$(CC) $(CPPFLAGS) $(OBJ) -o main $(LDFLAGS)

# Clean rule
clean:
	rm -rf $(OBJ) main

.PHONY: clean