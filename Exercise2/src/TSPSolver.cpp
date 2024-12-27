#include "TSPSolver.h"
#include "data_generator.h"
#include "visualization.h"
#include <limits>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>

const int TSPSolver::MAX_ITERATIONS_WITHOUT_IMPROVEMENT = 100;
const int TSPSolver::INTENSIFICATION_ITERATIONS = 50;
const int TSPSolver::MIN_MOVES_FOR_STATS = 10;
const double TSPSolver::IMPROVEMENT_THRESHOLD = 0.01;

TSPSolver::TSPSolver() :
    tabu_tenure(7), max_iterations(1000),
    min_tenure(5), max_tenure(20),
    iterations_without_improvement(0),
    best_known_value(std::numeric_limits<double>::max()),
    in_intensification_phase(false),
    best_intensification_solution(TSPSolution(TSP())) {
    srand(static_cast<unsigned>(time(nullptr)));
}

void TSPSolver::initializeMemoryStructures(int size) {
    frequency_matrix.clear();
    frequency_matrix.resize(size, std::vector<int>(size, 0));
    move_history.clear();
    search_history.clear();
    tabu_list.clear();
    best_intensification_value = std::numeric_limits<double>::max();
}

void TSPSolver::updateMoveFrequency(const Move& move, double improvement) {
    auto move_pair = std::make_pair(move.from, move.to);
    auto& freq = move_history[move_pair];
    freq.frequency++;
    freq.avg_improvement = (freq.avg_improvement * (freq.frequency - 1) + improvement) / freq.frequency;
}

void TSPSolver::updateSearchStats(int iteration, double current_value,
    double previous_value, double time_elapsed) {
    double improvement = previous_value - current_value;
    double improvement_percentage = (previous_value != 0) ?
        (improvement / previous_value) * 100.0 : 0.0;

    search_history.push_back({
        iteration,
        current_value,
        tabu_tenure,
        improvement > IMPROVEMENT_THRESHOLD,
        improvement_percentage,
        time_elapsed
        });
}

bool TSPSolver::solveWithTabuSearch(const TSP& tsp, const TSPSolution& initSol,
    TSPSolution& bestSol, const std::vector<std::pair<double, double>>& points,
    int save_every) {
    try {
        int iteration = 0;
        bool stop = false;
        auto start_time = std::chrono::high_resolution_clock::now();

        initializeMemoryStructures(tsp.n);

        TSPSolution currSol(initSol);
        double bestValue = evaluate(currSol, tsp);
        double currValue = bestValue;
        bestSol = currSol;
        best_known_value = bestValue;

        while (!stop) {
            double prev_value = currValue;
            Move move = findBestNeighbor(tsp, currSol, iteration);

            if (move.cost_change >= tsp.infinite) {
                if (in_intensification_phase) {
                    in_intensification_phase = false;
                    continue;
                }
                stop = true;
                continue;
            }

            updateTabuList(currSol.sequence[move.from], currSol.sequence[move.to], iteration);
            currSol = applyMove(currSol, move);
            currValue += move.cost_change;

            // Update statistics and memory structures
            updateMoveFrequency(move, prev_value - currValue);
            auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::high_resolution_clock::now() - start_time).count() / 1000.0;
            updateSearchStats(iteration, currValue, prev_value, current_time);

            // Adjust search strategy based on progress
            if (shouldIntensify(currValue, bestValue)) {
                bestValue = currValue;
                bestSol = currSol;
                in_intensification_phase = true;
                intensifySearch(tsp, currSol);
                currValue = evaluate(currSol, tsp);
            }
            else if (shouldDiversify()) {
                diversifySearch(currSol);
                currValue = evaluate(currSol, tsp);
            }

            if (iteration % save_every == 0) {
                BoardVisualizer::saveKeySnapshots(points, currSol.sequence,
                    "visualizations/solution", iteration, currValue);
            }

            adjustTabuTenure(currValue);

            iteration++;
            if (iteration >= max_iterations) {
                stop = true;
            }
        }

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

    int original_tenure = tabu_tenure;
    tabu_tenure = min_tenure;

    TSPSolution best_local = current_sol;
    double best_local_value = backup_value;

    for (int i = 0; i < INTENSIFICATION_ITERATIONS; i++) {
        Move move = findBestNeighbor(tsp, current_sol, i);
        if (move.cost_change >= tsp.infinite) break;

        current_sol = applyMove(current_sol, move);
        double new_value = evaluate(current_sol, tsp);

        updateMoveFrequency(move, backup_value - new_value);

        if (new_value < best_local_value) {
            best_local = current_sol;
            best_local_value = new_value;
        }

        updateTabuList(current_sol.sequence[move.from],
            current_sol.sequence[move.to], i);
    }

    tabu_tenure = original_tenure;

    if (best_local_value < backup_value) {
        current_sol = best_local;
        if (best_local_value < best_intensification_value) {
            best_intensification_solution = best_local;
            best_intensification_value = best_local_value;
        }
    }
    else {
        current_sol = backup;
    }
}

void TSPSolver::diversifySearch(TSPSolution& current_sol) {
    // Use frequency information to guide diversification
    std::vector<std::pair<int, int>> least_used_moves;

    for (size_t i = 0; i < frequency_matrix.size(); i++) {
        for (size_t j = i + 1; j < frequency_matrix.size(); j++) {
            if (frequency_matrix[i][j] < frequency_matrix.size() / 4) {
                least_used_moves.push_back({ i, j });
            }
        }
    }

    // Apply a series of less-frequently used moves
    int num_moves = current_sol.sequence.size() / 3;
    for (int i = 0; i < num_moves && !least_used_moves.empty(); i++) {
        int idx = rand() % least_used_moves.size();
        auto move = least_used_moves[idx];

        // Find positions in sequence
        auto it1 = std::find(current_sol.sequence.begin(),
            current_sol.sequence.end(), move.first);
        auto it2 = std::find(current_sol.sequence.begin(),
            current_sol.sequence.end(), move.second);

        if (it1 != current_sol.sequence.end() && it2 != current_sol.sequence.end()) {
            std::iter_swap(it1, it2);
        }

        least_used_moves.erase(least_used_moves.begin() + idx);
    }

    tabu_list.clear();
    iterations_without_improvement = 0;
}

bool TSPSolver::shouldIntensify(double current_value, double best_value) const {
    return current_value < best_value - IMPROVEMENT_THRESHOLD ||
        (iterations_without_improvement == 0 && current_value < best_value);
}

bool TSPSolver::shouldDiversify() const {
    return iterations_without_improvement >= MAX_ITERATIONS_WITHOUT_IMPROVEMENT ||
        (tabu_tenure >= max_tenure - 2 &&
            iterations_without_improvement >= MAX_ITERATIONS_WITHOUT_IMPROVEMENT / 2);
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

            double costChange = calculateMoveCost(tsp, currSol, Move(a, b));

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
    frequency_matrix[from][to]++;
    frequency_matrix[to][from]++;
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
    for (std::size_t i = 1; i < sol.sequence.size() - 1; i++) {
        int j = 1 + rand() % (sol.sequence.size() - 2);
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