/**
* @file TSPSolution.h
* @brief Solution representation for TSP
*
* Represents a TSP solution as a sequence of nodes (drilling points).
* Provides:
* - Construction from TSP instance
* - Deep copy functionality
* - Solution printing
* - Assignment operations
*
* The solution is represented as a vector of indices where:
* - First and last elements are always 0 (depot)
* - Intermediate elements represent the visiting sequence
*/

#ifndef TSPSOLUTION_H
#define TSPSOLUTION_H

#include <vector>
#include "TSP.h"

class TSPSolution {
public:
    std::vector<int> sequence;  // drilling sequence

    // Constructor with TSP instance reference
    TSPSolution(const TSP& tsp);

    // Copy constructor
    TSPSolution(const TSPSolution& tspSol);

    void print();
    TSPSolution& operator=(const TSPSolution& right);
};

#endif /* TSPSOLUTION_H */