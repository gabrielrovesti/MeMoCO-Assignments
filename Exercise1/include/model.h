/**
* @file model.h
* @brief Flow-based TSP model implementation for circuit board drilling optimization
*
* This module implements a compact flow-based formulation for solving the Traveling
* Salesman Problem (TSP) as applied to circuit board drilling optimization. The
* implementation uses CPLEX to solve the following model:
*
* Features:
* - Network flow formulation with subtour elimination via flow constraints
* - Compact model with polynomial number of variables and constraints
* - Based on Gavish & Graves (1978) formulation
*
* The model handles:
* - Flow variables (x_ij): Amount of flow from node i to j
* - Path variables (y_ij): Binary variables indicating selected arcs
* - Flow conservation constraints
* - Assignment constraints ensuring each node is visited exactly once
* - Linking constraints between flow and path variables
*/

#ifndef MODEL_H
#define MODEL_H

#include <ilcplex/cplex.h>  
#include <cpxmacro.h>       
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>

class TSPModel {
private:
    // Variable mappings
    std::vector<std::vector<int>> map_x;  // Flow variables x[i][j], j≠0 
    std::vector<std::vector<int>> map_y;  // Path variables y[i][j]

    void setupVariables(CEnv env, Prob lp, int N, const std::vector<std::vector<double>>& costs);
    void setupConstraints(CEnv env, Prob lp, int N);
    void setupFlowConservation(CEnv env, Prob lp, int N);
    void setupAssignmentConstraints(CEnv env, Prob lp, int N);
    void setupLinkingConstraints(CEnv env, Prob lp, int N);

public:
    TSPModel() = default;
    void createModel(CEnv env, Prob lp, int N, const std::vector<std::vector<double>>& costs);
    bool solve(CEnv env, Prob lp, double& objval, std::vector<int>& tour);
    void printSolution(const std::vector<double>& solution, int N);
};

#endif