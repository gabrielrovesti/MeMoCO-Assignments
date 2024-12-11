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