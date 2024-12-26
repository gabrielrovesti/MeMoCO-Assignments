#include "TSPSolver.h"
#include "data_generator.h"
#include "visualization.h"
#include <limits>
#include <cstdlib>
#include <ctime>

TSPSolver::TSPSolver() :
    tabu_tenure(7), max_iterations(1000),
    min_tenure(5), max_tenure(20),
    iterations_without_improvement(0),
    best_known_value(std::numeric_limits<double>::max()),
    in_intensification_phase(false) {}

bool TSPSolver::solveWithTabuSearch(const TSP& tsp, const TSPSolution& initSol,
    TSPSolution& bestSol, const std::vector<std::pair<double, double>>& points,
    int save_every) {
    try {
        int iteration = 0;
        bool stop = false;

        TSPSolution currSol(initSol);
        double bestValue = evaluate(currSol, tsp);
        double currValue = bestValue;
        bestSol = currSol;
        best_known_value = bestValue;

        // Initialize tabu list
        tabu_list.clear();
        search_history.clear();

        // Save initial solution
        BoardVisualizer::saveKeySnapshots(points, currSol.sequence,
            "visualizations/solution", iteration, currValue);

        while (!stop) {
            // Find best neighbor
            Move move = findBestNeighbor(tsp, currSol, iteration);

            if (move.cost_change >= tsp.infinite) {
                if (in_intensification_phase) {
                    in_intensification_phase = false;
                    continue;
                }
                stop = true;
                continue;
            }

            // Update tabu list and apply move
            updateTabuList(currSol.sequence[move.from], currSol.sequence[move.to], iteration);
            currSol = applyMove(currSol, move);
            currValue += move.cost_change;

            // Track search progress
            search_history.push_back({
                iteration, currValue, tabu_tenure,
                currValue < bestValue - 0.01
                });

            // Adjust search strategy
            if (shouldIntensify(currValue, bestValue)) {
                bestValue = currValue;
                bestSol = currSol;
                in_intensification_phase = true;
                intensifySearch(tsp, currSol);
                currValue = evaluate(currSol, tsp);

                // Save visualization of improvement
                BoardVisualizer::saveKeySnapshots(points, currSol.sequence,
                    "visualizations/solution", iteration, currValue);
            }
            else if (shouldDiversify()) {
                diversifySearch(currSol);
                currValue = evaluate(currSol, tsp);
                BoardVisualizer::saveKeySnapshots(points, currSol.sequence,
                    "visualizations/solution", iteration, currValue);
            }
            else if (iteration % save_every == 0) {
                BoardVisualizer::saveKeySnapshots(points, currSol.sequence,
                    "visualizations/solution", iteration, currValue);
            }

            // Update reactive parameters
            adjustTabuTenure(currValue);

            // Check stopping criteria
            iteration++;
            if (iteration >= max_iterations) {
                stop = true;
            }
        }

        // Save final solution
        BoardVisualizer::saveKeySnapshots(points, bestSol.sequence,
            "visualizations/solution", iteration, bestValue);

        return true;
    }
    catch (std::exception& e) {
        std::cout << ">>>EXCEPTION in Tabu Search: " << e.what() << std::endl;
        return false;
    }
}

void TSPSolver::adjustTabuTenure(double current_value) {
    if (current_value >= best_known_value) {
        iterations_without_improvement++;
        if (iterations_without_improvement > MAX_ITERATIONS_WITHOUT_IMPROVEMENT / 2) {
            tabu_tenure = std::min(tabu_tenure + 2, max_tenure);
        }
    }
    else {
        tabu_tenure = std::max(tabu_tenure - 1, min_tenure);
        iterations_without_improvement = 0;
        best_known_value = current_value;
    }
}

void TSPSolver::intensifySearch(const TSP& tsp, TSPSolution& current_sol) {
    TSPSolution backup = current_sol;
    double backup_value = evaluate(backup, tsp);

    // Perform intensified local search with shorter tabu tenure
    int original_tenure = tabu_tenure;
    tabu_tenure = min_tenure;

    for (int i = 0; i < INTENSIFICATION_ITERATIONS; i++) {
        Move move = findBestNeighbor(tsp, current_sol, i);
        if (move.cost_change >= tsp.infinite) break;

        current_sol = applyMove(current_sol, move);
        updateTabuList(current_sol.sequence[move.from],
            current_sol.sequence[move.to], i);
    }

    // Restore original tenure and best solution if no improvement
    tabu_tenure = original_tenure;
    if (evaluate(current_sol, tsp) > backup_value) {
        current_sol = backup;
    }
}

void TSPSolver::diversifySearch(TSPSolution& current_sol) {
    // Perform multiple random moves
    int num_diversification_moves = current_sol.sequence.size() / 3;
    for (int i = 0; i < num_diversification_moves; i++) {
        int idx1 = 1 + rand() % (current_sol.sequence.size() - 2);
        int idx2 = 1 + rand() % (current_sol.sequence.size() - 2);
        std::swap(current_sol.sequence[idx1], current_sol.sequence[idx2]);
    }
    // Clear tabu list after diversification
    tabu_list.clear();
}

bool TSPSolver::shouldIntensify(double current_value, double best_value) const {
    return current_value < best_value - 0.01;
}

bool TSPSolver::shouldDiversify() const {
    return iterations_without_improvement >= MAX_ITERATIONS_WITHOUT_IMPROVEMENT;
}

TSPSolver::Move TSPSolver::findBestNeighbor(const TSP& tsp, const TSPSolution& currSol,
    int iteration) {
    Move bestMove;
    bestMove.cost_change = tsp.infinite;

    for (std::size_t a = 1; a < currSol.sequence.size() - 2; a++) {
        int h = currSol.sequence[a - 1];
        int i = currSol.sequence[a];

        for (std::size_t b = a + 1; b < currSol.sequence.size() - 1; b++) {
            int j = currSol.sequence[b];
            int l = currSol.sequence[b + 1];

            if (isTabu(i, j, iteration)) {
                continue;
            }

            double costChange = -tsp.cost[h][i] - tsp.cost[j][l]
                + tsp.cost[h][j] + tsp.cost[i][l];

            if (costChange < bestMove.cost_change) {
                bestMove.from = static_cast<int>(a);
                bestMove.to = static_cast<int>(b);
                bestMove.cost_change = costChange;
            }
        }
    }
    return bestMove;
}

void TSPSolver::updateTabuList(int from, int to, int iteration) {
    tabu_list.push_back(std::make_pair(from, to));
    if (tabu_list.size() > tabu_tenure) {
        tabu_list.pop_front();
    }
}

bool TSPSolver::isTabu(int from, int to, int iteration) {
    for (const auto& move : tabu_list) {
        if ((move.first == from && move.second == to) ||
            (move.first == to && move.second == from)) {
            return true;
        }
    }
    return false;
}

double TSPSolver::evaluate(const TSPSolution& sol, const TSP& tsp) const {
    double total = 0.0;
    for (std::size_t i = 0; i < sol.sequence.size() - 1; i++) {
        int from = sol.sequence[i];
        int to = sol.sequence[i + 1];
        total += tsp.cost[from][to];
    }
    return total;
}

bool TSPSolver::initRnd(TSPSolution& sol) {
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned>(time(nullptr)));
        seeded = true;
    }

    for (std::size_t i = 1; i < sol.sequence.size() - 1; i++) {
        int j = rand() % (sol.sequence.size() - 2) + 1;
        std::swap(sol.sequence[i], sol.sequence[j]);
    }
    return true;
}

TSPSolution& TSPSolver::applyMove(TSPSolution& tspSol, const Move& move) {
    int left = move.from;
    int right = move.to;
    while (left < right) {
        std::swap(tspSol.sequence[left], tspSol.sequence[right]);
        left++;
        right--;
    }
    return tspSol;
}

double TSPSolver::calculateMoveCost(const TSP& tsp, const TSPSolution& sol,
    const Move& move) {
    int h = sol.sequence[move.from - 1];
    int i = sol.sequence[move.from];
    int j = sol.sequence[move.to];
    int l = sol.sequence[move.to + 1];

    return -tsp.cost[h][i] - tsp.cost[j][l] + tsp.cost[h][j] + tsp.cost[i][l];
}