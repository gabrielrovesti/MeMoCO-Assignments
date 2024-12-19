/**
* @file TSP.h
* @brief Traveling Salesman Problem instance representation
*
* Represents a TSP instance with its cost matrix and basic properties.
* While it includes a read() method for file input, the current implementation
* primarily uses the TSPGenerator for instance creation.
*
* @note The read() method is currently not actively used but kept for potential
* future file-based instance handling.
*/

#ifndef TSP_H
#define TSP_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

class TSP {
public:
    TSP() : n(0), infinite(1e10) {}
    int n;  // number of nodes/holes
    std::vector<std::vector<double>> cost;  // cost matrix
    double infinite;  // upper bound value for invalid solutions
};

#endif /* TSP_H */