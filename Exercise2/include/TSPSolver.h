#ifndef TSPSOLVER_H
#define TSPSOLVER_H

#include <vector>
#include <deque>
#include <limits>
#include "TSPSolution.h"
#include "TSP.h"

class TSPSolver {
public:
    // Constructor with enhanced initialization
    TSPSolver();

    // Core methods
    double evaluate(const TSPSolution& sol, const TSP& tsp) const;
    bool initRnd(TSPSolution& sol);
    bool solveWithTabuSearch(const TSP& tsp, const TSPSolution& initSol, 
                           TSPSolution& bestSol,
                           const std::vector<std::pair<double, double>>& points,
                           int save_every = 100);

    // Configuration methods
    void setTabuTenure(int tenure) { tabu_tenure = tenure; }
    void setMaxIterations(int iterations) { max_iterations = iterations; }

protected:
    // Search statistics and control
    struct SearchStats {
        int iteration;
        double solution_value;
        int current_tenure;
        bool was_improvement;
    };

    // Move structure
    struct Move {
        int from;
        int to;
        double cost_change;
        Move(int f = -1, int t = -1, double cost = 0.0)
            : from(f), to(t), cost_change(cost) {}
    };

    // Core parameters
    int tabu_tenure;
    int max_iterations;
    std::deque<std::pair<int, int>> tabu_list;

    // Reactive parameters
    int min_tenure;
    int max_tenure;
    int iterations_without_improvement;
    double best_known_value;
    bool in_intensification_phase;
    std::vector<SearchStats> search_history;

    // Constants for search control
    static const int MAX_ITERATIONS_WITHOUT_IMPROVEMENT = 100;
    static const int INTENSIFICATION_ITERATIONS = 50;

    // Core helper methods
    Move findBestNeighbor(const TSP& tsp, const TSPSolution& currSol, int iteration);
    bool isTabu(int from, int to, int iteration);
    void updateTabuList(int from, int to, int iteration);
    TSPSolution& applyMove(TSPSolution& tspSol, const Move& move);
    double calculateMoveCost(const TSP& tsp, const TSPSolution& sol, const Move& move);

    // Reactive search methods
    void adjustTabuTenure(double current_value);
    void intensifySearch(const TSP& tsp, TSPSolution& current_sol);
    void diversifySearch(TSPSolution& current_sol);
    bool shouldIntensify(double current_value, double best_value) const;
    bool shouldDiversify() const;
};

#endif /* TSPSOLVER_H */