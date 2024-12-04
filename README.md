# MeMoCO-Assignments
First and second exam assignments for the Methods and Models for Combinatorial Optimization (MeMoCO) course at the University of Padova.

# 1 Assignment - Circuit Board Drilling Optimization

## Project Overview
This project implements an optimization solution for the circuit board drilling problem using Mixed Integer Linear Programming (MILP). The goal is to determine the optimal sequence for drilling holes in circuit boards to minimize the total travel time of the drilling machine. The problem is modeled as a variant of the Traveling Salesman Problem (TSP) using a flow-based formulation from Gavish & Graves (1978).

## Mathematical Formulation
The problem is modeled on a complete weighted graph G = (N, A), where:
- N represents the set of hole positions
- A represents the possible drill movements between holes
- cij represents the time/cost to move from hole i to j

The implemented mathematical model minimizes the total drilling path cost:

```
Min Σ(i,j∈A) cij * yij                                 (9)
s.t.
Σ(i:(i,k)∈A) xik - Σ(j:(k,j),j≠0) xkj = 1            (10)
Σ(j:(i,j)∈A) yij = 1                                  (11)
Σ(i:(i,j)∈A) yij = 1                                  (12)
xij ≤ (|N|-1)yij                     ∀(i,j)∈A, j≠0    (13)
xij ∈ R+                             ∀(i,j)∈A, j≠0    (14)
yij ∈ {0,1}                          ∀(i,j)∈A         (15)
```

## Project Structure
```
.
├── include/
│   ├── cpxmacro.h       # CPLEX helper macros
│   ├── data_generator.h # Instance generation utilities
│   └── model.h         # TSP model class definition
├── src/
│   ├── main.cpp        # Main program
│   └── model.cpp       # Model implementation
├── CMakeLists.txt      # CMake build configuration
└── Makefile           # Make build configuration
```

## Requirements
- IBM ILOG CPLEX Optimization Studio 22.1.1
- C++11 compatible compiler
- CMake 3.10 or higher (if using CMake build)
- Linux environment (tested on x86-64)

## Build Instructions

### Using Make
1. Edit the `Makefile` to set your CPLEX installation path:
```bash
CPLEX_DIR = /path/to/cplex
CONCERT_DIR = /path/to/concert
```

2. Build the project:
```bash
make
```

### Using CMake
1. Edit `CMakeLists.txt` to set your CPLEX path:
```cmake
set(CPLEX_ROOT_DIR "/path/to/cplex" CACHE PATH "CPLEX root directory")
```

2. Build the project:
```bash
mkdir build
cd build
cmake ..
make
```

## Usage
The program can be run with:
```bash
./tsp_solver
```

The solver will test different board configurations:
- Small boards (~15-20 holes)
- Medium boards (~25-35 holes)
- Large boards (~40-50 holes)

Results include:
- Solution time
- Total drilling path length
- Complete drilling sequence
- Generated instance files in data/{small,medium,large} directories

## Performance Analysis
The implementation has been tested with various problem sizes:
- Small instances (≤20 holes): Solved optimally within 0.1-1 seconds
- Medium instances (21-35 holes): Solved optimally within 1-10 seconds
- Large instances (36-50 holes): May require several minutes, with a 5-minute time limit

## Documentation
- Source code is documented using Doxygen-style comments
- The mathematical model follows the formulation from Gavish & Graves (1978)
- Generated instances include metadata about board configuration

## Contributing
When contributing to this project:
1. Ensure all code follows the existing style
2. Document new features and changes
3. Add appropriate test cases
4. Update the README as needed

## References
Gavish, B., & Graves, S. C. (1978). The travelling salesman problem and related problems. Working Paper OR-078-78. Operations Research Center, Massachusetts Institute of Technology, Cambridge.