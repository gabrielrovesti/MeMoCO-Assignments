#ifndef TSPSOLVER_H
#define TSPSOLVER_H

#include <vector>
#include "TSPSolution.h"
#include "TSPMove.h"

class TSPSolver {
public:
    TSPSolver() {}

    // Solution evaluation
    double evaluate(const TSPSolution& sol, const TSP& tsp) const;

    // Initialize random solution
    bool initRnd(TSPSolution& sol);

    // Main solving method
    bool solve(const TSP& tsp, const TSPSolution& initSol, int tabuLength, int maxIter, TSPSolution& bestSol);

protected:
    double findBestNeighbor(const TSP& tsp, const TSPSolution& currSol, int currIter, TSPMove& move);
    TSPSolution& swap(TSPSolution& tspSol, const TSPMove& move);

    // Tabu list management
    int tabuLength;
    std::vector<int> tabuList;
    void initTabuList(int n);
    void updateTabuList(int nodeFrom, int nodeTo, int iter);
    bool isTabu(int nodeFrom, int nodeTo, int iter);
};

#endif /* TSPSOLVER_H */