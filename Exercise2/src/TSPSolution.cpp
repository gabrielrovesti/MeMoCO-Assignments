#include "TSPSolution.h"
#include <iostream>

TSPSolution::TSPSolution(const TSP& tsp) {
    // Initialize sequence from 0 to n-1, representing a simple initial tour
    sequence.reserve(tsp.n + 1);
    for (int i = 0; i < tsp.n; i++) {
        sequence.push_back(i);
    }
    sequence.push_back(0);  // Return to starting point
}

TSPSolution::TSPSolution(const TSPSolution& tspSol) {
    sequence = tspSol.sequence;
}

void TSPSolution::print() {
    for (std::size_t i = 0; i < sequence.size(); i++) {
        std::cout << sequence[i];
        if (i < sequence.size() - 1) std::cout << " -> ";
    }
}

TSPSolution& TSPSolution::operator=(const TSPSolution& right) {
    if (this != &right) {
        sequence = right.sequence;
    }
    return *this;
}