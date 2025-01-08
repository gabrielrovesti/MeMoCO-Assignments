# Circuit Board Drilling Optimization
This project implements and compares exact (CPLEX) and metaheuristic (Tabu Search) approaches for optimizing drilling sequences on PCB (Printed Circuit Board) manufacturing.

## Problem Description
A manufacturing company produces circuit boards that require precise hole drilling. A drilling machine moves across the board surface, stops at predetermined positions, and creates holes. Once a board is completely drilled, it's replaced with another one and the process repeats. The goal is to minimize the total drilling time by determining the optimal sequence of drilling operations, considering fixed drilling time per hole.

The problem is modeled as a Traveling Salesman Problem (TSP) where:
- Nodes represent hole positions 
- Edges represent drill movement paths
- Edge weights represent movement time between holes
- The objective is to find the minimum-weight Hamiltonian cycle

## Part 1: Exact Method (CPLEX)
The first part implements an exact solution using Mixed Integer Linear Programming (MILP) with CPLEX:

- Uses Gavish-Graves flow-based TSP formulation
- Provides optimality guarantees for small instances
- Solves small boards (≤50 holes) optimally within seconds
- Shows exponential runtime growth for larger instances

### Project Structure
```
part1/
├── include/
│   ├── cpxmacro.h            # CPLEX helper macros
│   ├── data_generator.h      # Instance generation utilities
│   └── model.h               # TSP model definition
├── src/
│   ├── main.cpp              # Program entry point
│   ├── model.cpp             # Model implementation 
├── data/                     # Generated test instances
├── results/                  # Performance analysis
├── Makefile
└── README.md
```

## Part 2: Metaheuristic Method (Tabu Search)
The second part implements an advanced Tabu Search metaheuristic:

- Uses 2-opt neighborhood structure
- Incorporates dynamic tabu tenure adjustment
- Features intensification and diversification strategies 
- Achieves near-optimal solutions quickly
- Scales effectively to large instances

### Project Structure
```
part2/
├── include/
│   ├── TSPSolver.h           # Tabu Search implementation
│   ├── TSPSolution.h         # Solution representation
│   ├── parameter_calibration.h  # Parameter tuning
│   └── visualization.h       # Solution visualization
├── src/
│   ├── main.cpp              # Program entry point
│   ├── TSPSolver.cpp         # Core algorithm
│   ├── parameter_calibration.cpp 
├── data/                     # Test instances
├── results/                  # Analysis output
├── visualizations/           # Solution diagrams
├── Makefile
└── README.md
```

## Build Instructions

### Prerequisites
- IBM ILOG CPLEX Optimization Studio (22.1.1 or higher)
- C++11 compatible compiler
- CMake 3.10+ (optional)
- Linux environment (tested on x86-64)

### Compilation 
For both parts, execute in order:
```bash
make clean       # Clean previous build
# or
make distclean  # Full clean including data/results

make            # Build project

./build/bin/ilolpex1  # Run executable
```

## Results Summary
- Small instances (≤20 holes): Both methods find optimal solutions, CPLEX faster
- Medium instances (21-35 holes): CPLEX optimal but slower, TS near-optimal quickly  
- Large instances (>35 holes): TS clearly superior in runtime and scalability
- Solution quality: TS achieves 61-84% improvement vs initial solutions
- Runtime scaling: TS maintains reasonable times even for 100+ holes

## Directory Structure
```
.
├── Exercise1/                    # Exact method implementation  
├── Exercise2/                    # Tabu Search implementation
├── Report/                       # Report of the project
├── References/                   # Texts of both assignments
└── README.md
```

## Acknowledgments
Based on:
- Gavish & Graves (1978) TSP formulation
- Fred Glover's Tabu Search metaheuristic
- Lin-Kernighan local search heuristics

## References
1. Gavish, B., & Graves, S. C. (1978). The travelling salesman problem and related problems.
2. Glover, F. (1989). Tabu Search—Part I.
3. Lin, S., & Kernighan, B. W. (1973). An effective heuristic algorithm for the TSP.