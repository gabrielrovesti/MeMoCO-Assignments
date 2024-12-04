// model.cpp
#include <cpxmacro.h>
#include <ilcplex/cplex.h>
#include <model.h>
#include <iostream>
#include <iomanip>

// PCB Manufacturing Constants
const double DRILL_SPEED = 50.0;      // mm/sec average drill movement speed
const double DRILL_TIME = 0.5;        // seconds per hole
const double MIN_SPACING = 0.8;       // minimum hole spacing in mm
const double BOARD_THICKNESS = 1.6;   // standard PCB thickness

void TSPModel::setupVariables(CEnv env, Prob lp, int N, const std::vector<std::vector<double>>& costs) {
    int current_var_position = 0;

    // Initialize mappings
    map_x.resize(N, std::vector<int>(N, -1));
    map_y.resize(N, std::vector<int>(N, -1));

    // Create flow variables x[i][j] - only for j≠0
    for (int i = 0; i < N; i++) {
        for (int j = 1; j < N; j++) {
            if (i != j) {
                char xtype = 'C';
                double lb = 0.0;
                double ub = CPX_INFBOUND;
                char* name = new char[50];  // Allocate memory for the name
                snprintf(name, 50, "x_%d_%d", i, j);  // Create the name string
                double obj = 0.0;  // Flow variables not in objective

                // Make the CPLEX call with properly typed parameters
                CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &obj, &lb, &ub, &xtype, &name);
                delete[] name;  // Clean up allocated memory

                map_x[i][j] = current_var_position++;
            }
        }
    }

    // Create path variables y[i][j]
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i != j) {
                char ytype = 'B';
                double lb = 0.0;
                double ub = 1.0;
                char* name = new char[50];
                snprintf(name, 50, "y_%d_%d", i, j);
                double obj = costs[i][j];

                CHECKED_CPX_CALL(CPXnewcols, env, lp, 1, &obj, &lb, &ub, &ytype, &name);
                delete[] name;

                map_y[i][j] = current_var_position++;
            }
        }
    }
}

void TSPModel::setupFlowConservation(CEnv env, Prob lp, int N) {
    for (int k = 1; k < N; k++) {
        std::vector<int> idx;
        std::vector<double> coef;

        // Incoming flows
        for (int i = 0; i < N; i++) {
            if (i != k && map_x[i][k] >= 0) {
                idx.push_back(map_x[i][k]);
                coef.push_back(1.0);
            }
        }

        // Outgoing flows (j≠0)
        for (int j = 1; j < N; j++) {
            if (j != k && map_x[k][j] >= 0) {
                idx.push_back(map_x[k][j]);
                coef.push_back(-1.0);
            }
        }

        char sense = 'E';
        double rhs = 1.0;
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL, NULL);
    }
}

void TSPModel::setupAssignmentConstraints(CEnv env, Prob lp, int N) {
    // One outgoing arc
    for (int i = 0; i < N; i++) {
        std::vector<int> idx;
        std::vector<double> coef;
        for (int j = 0; j < N; j++) {
            if (i != j && map_y[i][j] >= 0) {
                idx.push_back(map_y[i][j]);
                coef.push_back(1.0);
            }
        }
        char sense = 'E';
        double rhs = 1.0;
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL, NULL);
    }

    // One incoming arc
    for (int j = 0; j < N; j++) {
        std::vector<int> idx;
        std::vector<double> coef;
        for (int i = 0; i < N; i++) {
            if (i != j && map_y[i][j] >= 0) {
                idx.push_back(map_y[i][j]);
                coef.push_back(1.0);
            }
        }
        char sense = 'E';
        double rhs = 1.0;
        int matbeg = 0;
        CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense, &matbeg, &idx[0], &coef[0], NULL, NULL);
    }
}

void TSPModel::setupLinkingConstraints(CEnv env, Prob lp, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 1; j < N; j++) {
            if (i != j && map_x[i][j] >= 0 && map_y[i][j] >= 0) {
                // Create index vector for the two variables involved
                std::vector<int> idx = { map_x[i][j], map_y[i][j] };

                // Fix: Cast N to double before arithmetic to avoid conversion error
                double bigN = static_cast<double>(N);
                std::vector<double> coef = { 1.0, -(bigN - 1.0) };

                char sense = 'L';
                double rhs = 0.0;
                int matbeg = 0;

                CHECKED_CPX_CALL(CPXaddrows, env, lp, 0, 1, idx.size(), &rhs, &sense,
                    &matbeg, &idx[0], &coef[0], NULL, NULL);
            }
        }
    }
}

void TSPModel::setupConstraints(CEnv env, Prob lp, int N) {
    setupFlowConservation(env, lp, N);
    setupAssignmentConstraints(env, lp, N);
    setupLinkingConstraints(env, lp, N);
}

void TSPModel::createModel(CEnv env, Prob lp, int N, const std::vector<std::vector<double>>& costs) {
    setupVariables(env, lp, N, costs);
    setupConstraints(env, lp, N);
}

bool TSPModel::solve(CEnv env, Prob lp, double& objval, std::vector<int>& tour) {
    CHECKED_CPX_CALL(CPXmipopt, env, lp);

    // Get objective value
    CHECKED_CPX_CALL(CPXgetobjval, env, lp, &objval);

    // Get solution values
    int n = CPXgetnumcols(env, lp);
    std::vector<double> x(n);
    CHECKED_CPX_CALL(CPXgetx, env, lp, &x[0], 0, n - 1);

    // Extract tour
    tour.clear();
    tour.push_back(0);  // Start at depot
    int current = 0;
    int N = map_y.size();

    for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < N; j++) {
            if (current != j && map_y[current][j] >= 0 && x[map_y[current][j]] > 0.5) {
                tour.push_back(j);
                current = j;
                break;
            }
        }
    }

    return true;
}

void TSPModel::printSolution(const std::vector<double>& solution, int N) {
    std::cout << "\nPath variables (y):" << std::endl;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i != j && map_y[i][j] >= 0) {
                if (solution[map_y[i][j]] > 0.5) {
                    std::cout << "y_" << i << "_" << j << " = 1" << std::endl;
                }
            }
        }
    }
}