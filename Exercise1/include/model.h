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