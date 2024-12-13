#ifndef TSPSOLVER_H
#define TSPSOLVER_H

#include <vector>
#include <deque>
#include "TSPSolution.h"
#include "TSP.h"

class TSPSolver {
public:
    // Existing constructors and methods
    TSPSolver() : tabu_tenure(7), max_iterations(1000) {}

    // Original evaluation method
    double evaluate(const TSPSolution& sol, const TSP& tsp) const;
    bool initRnd(TSPSolution& sol);

    // New Tabu Search methods
    bool solveWithTabuSearch(const TSP& tsp, const TSPSolution& initSol, TSPSolution& bestSol);

    // Configuration methods
    void setTabuTenure(int tenure) { tabu_tenure = tenure; }
    void setMaxIterations(int iterations) { max_iterations = iterations; }

protected:
    // Tabu Search components
    struct Move {
        int from;
        int to;
        double cost_change;

        Move(int f = -1, int t = -1, double cost = 0.0)
            : from(f), to(t), cost_change(cost) {}
    };

    // Tabu Search parameters
    int tabu_tenure;
    int max_iterations;
    std::deque<std::pair<int, int>> tabu_list;

    // Helper methods
    Move findBestNeighbor(const TSP& tsp, const TSPSolution& currSol, int iteration);
    bool isTabu(int from, int to, int iteration);
    void updateTabuList(int from, int to, int iteration);
    TSPSolution& applyMove(TSPSolution& tspSol, const Move& move);
    double calculateMoveCost(const TSP& tsp, const TSPSolution& sol, const Move& move);
};

#endif