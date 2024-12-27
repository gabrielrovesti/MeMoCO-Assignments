#ifndef TSPSOLVER_H
#define TSPSOLVER_H

#include <vector>
#include <deque>
#include <limits>
#include <map>
#include "TSPSolution.h"
#include "TSP.h"

class TSPSolver {
public:
    TSPSolver();
    double evaluate(const TSPSolution& sol, const TSP& tsp) const;
    bool initRnd(TSPSolution& sol);
    bool solveWithTabuSearch(const TSP& tsp, const TSPSolution& initSol,
        TSPSolution& bestSol,
        const std::vector<std::pair<double, double>>& points,
        int save_every = 100);

    void setTabuTenure(int tenure) { tabu_tenure = tenure; }
    void setMaxIterations(int iterations) { max_iterations = iterations; }

protected:
    // Enhanced search statistics
    struct SearchStats {
        int iteration;
        double solution_value;
        int current_tenure;
        bool was_improvement;
        double improvement_percentage;
        double time_elapsed;
    };

    // Move tracking structure
    struct MoveFrequency {
        int from, to;
        int frequency;
        double avg_improvement;
        MoveFrequency() : from(-1), to(-1), frequency(0), avg_improvement(0.0) {}
    };

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
    std::map<std::pair<int, int>, MoveFrequency> move_history;

    // Reactive parameters
    int min_tenure;
    int max_tenure;
    int iterations_without_improvement;
    double best_known_value;
    bool in_intensification_phase;
    std::vector<SearchStats> search_history;

    // Memory structures
    std::vector<std::vector<int>> frequency_matrix;
    TSPSolution best_intensification_solution;
    double best_intensification_value;

    // Constants
    static const int MAX_ITERATIONS_WITHOUT_IMPROVEMENT;
    static const int INTENSIFICATION_ITERATIONS;
    static const int MIN_MOVES_FOR_STATS;
    static const double IMPROVEMENT_THRESHOLD;

    // Core methods
    Move findBestNeighbor(const TSP& tsp, const TSPSolution& currSol, int iteration);
    bool isTabu(int from, int to, int iteration);
    void updateTabuList(int from, int to, int iteration);
    TSPSolution& applyMove(TSPSolution& tspSol, const Move& move);
    double calculateMoveCost(const TSP& tsp, const TSPSolution& sol, const Move& move);

    // Enhanced reactive methods
    void adjustTabuTenure(double current_value);
    void intensifySearch(const TSP& tsp, TSPSolution& current_sol);
    void diversifySearch(TSPSolution& current_sol);
    bool shouldIntensify(double current_value, double best_value) const;
    bool shouldDiversify() const;

    // New helper methods
    void initializeMemoryStructures(int size);
    void updateMoveFrequency(const Move& move, double improvement);
    void updateSearchStats(int iteration, double current_value,
        double previous_value, double time_elapsed);
};

#endif /* TSPSOLVER_H */