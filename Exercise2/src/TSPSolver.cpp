#include "TSPSolver.h"
#include "data_generator.h"

bool TSPSolver::solveWithTabuSearch(const TSP& tsp, const TSPSolution& initSol, TSPSolution& bestSol) {
    try {
        int iteration = 0;
        bool stop = false;

        TSPSolution currSol(initSol);
        double bestValue = evaluate(currSol, tsp);
        double currValue = bestValue;
        bestSol = currSol;

        // Initialize tabu list
        tabu_list.clear();

        while (!stop) {
            // Find best non-tabu neighbor or one that meets aspiration criteria
            Move move = findBestNeighbor(tsp, currSol, iteration);

            if (move.cost_change >= tsp.infinite) {
                stop = true;
                continue;
            }

            // Update tabu list
            updateTabuList(currSol.sequence[move.from], currSol.sequence[move.to], iteration);

            // Apply move
            currSol = applyMove(currSol, move);
            currValue += move.cost_change;

            // Update best solution if improved
            if (currValue < bestValue - 0.01) {
                bestValue = currValue;
                bestSol = currSol;
            }

            // Stopping criteria
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

TSPSolver::Move TSPSolver::findBestNeighbor(const TSP& tsp, const TSPSolution& currSol, int iteration) {
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
    // Initialize random seed
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned>(time(nullptr)));
        seeded = true;
    }

    // Perform random swaps
    for (std::size_t i = 1; i < sol.sequence.size() - 1; i++) {
        int j = rand() % (sol.sequence.size() - 2) + 1;
        std::swap(sol.sequence[i], sol.sequence[j]);
    }
    return true;
}

TSPSolution& TSPSolver::applyMove(TSPSolution& tspSol, const Move& move) {
    // Reverse the subsequence between from and to
    int left = move.from;
    int right = move.to;
    while (left < right) {
        std::swap(tspSol.sequence[left], tspSol.sequence[right]);
        left++;
        right--;
    }
    return tspSol;
}

double TSPSolver::calculateMoveCost(const TSP& tsp, const TSPSolution& sol, const Move& move) {
    // Calculate the cost difference for a 2-opt move
    int h = sol.sequence[move.from - 1];
    int i = sol.sequence[move.from];
    int j = sol.sequence[move.to];
    int l = sol.sequence[move.to + 1];

    return -tsp.cost[h][i] - tsp.cost[j][l] + tsp.cost[h][j] + tsp.cost[i][l];
}