/**
 * @file main.cpp
 * @brief Circuit Board Drilling Optimization Implementation with CPLEX
 *
 * This program implements an exact solution approach for optimizing drilling sequences
 * on circuit boards using CPLEX. It handles multiple board configurations and provides
 * detailed performance analysis.
 *
 * Features:
 * - Automated instance generation for different board sizes
 * - Hierarchical data organization by instance size
 * - Comprehensive performance metrics tracking
 * - Manufacturing constraint validation
 *
 * Program Flow:
 * 1. Instance Generation:
 *    - Creates test boards of varying sizes (50x50 to 200x200)
 *    - Applies industry-standard manufacturing constraints
 *    - Categorizes instances by complexity (small/medium/large)
 *
 * 2. Solution Process:
 *    - Configures CPLEX parameters based on instance size
 *    - Implements adaptive time limits (10s-300s)
 *    - Tracks setup and solution times separately
 *
 * 3. Results Management:
 *    - Organizes results in hierarchical directory structure
 *    - Generates detailed performance metrics
 *    - Provides manufacturing-relevant statistics
 *
 * Usage:
 * The program automatically processes multiple board configurations:
 * - Small boards (50x50, ~10-15 holes)
 * - Medium boards (100x100, ~25-30 holes)
 * - Large boards (150x150, ~45-50 holes)
 * - Extra large boards (200x200, ~80-85 holes)
 *
 * Performance Metrics Generated:
 * - Model setup time
 * - Solution time
 * - Total drilling path length
 * - Average distance between holes
 * - Complete drilling sequence
 *
 * @note Directory structure is automatically created and managed
 * @note Error handling is implemented for both file operations and solver execution
 */

#include <iostream>
#include <iomanip>
#include <cpxmacro.h>
#include <model.h>
#include <data_generator.h>
#include <chrono>
#include <tuple>
#include <direct.h>
#include <fstream>

int status;
char errmsg[BUF_SIZE];

bool createDirectoryIfNeeded(const std::string& path) {
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
}

std::string getSizeCategory(int N) {
    if (N <= 20) return "small";
    if (N <= 35) return "medium";
    return "large";
}

int main(int argc, char const* argv[]) {
    try {
        std::vector<std::tuple<int, int, int>> board_configs = {
            {50, 50, 2},     // Small boards
            {75, 75, 3},     // Medium-small boards
            {100, 100, 3},   // Medium boards
            {125, 125, 4},   // Medium-large boards
            {150, 150, 5}    // Large boards
        };

        if (!createDirectoryIfNeeded("data")) {
            std::cerr << "Failed to create data directory" << std::endl;
            return 1;
        }

        for (const auto& folder : { "small", "medium", "large" }) {
            if (!createDirectoryIfNeeded("data/" + std::string(folder))) {
                std::cerr << "Failed to create " << folder << " directory" << std::endl;
                return 1;
            }
        }

        for (const auto& config : board_configs) {
            int width = std::get<0>(config);
            int height = std::get<1>(config);
            int components = std::get<2>(config);

            std::cout << "\n=== Testing circuit board " << width << "x" << height
                << " with " << components << " components ===\n\n";

            for (int instance = 0; instance < 10; instance++) {
                auto costs = TSPGenerator::generateCircuitBoard(width, height, components);
                int N = costs.size();
                std::string category = getSizeCategory(N);

                auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                std::string filename = "data/" + category + "/board_" +
                    std::to_string(width) + "x" +
                    std::to_string(height) + "_" +
                    std::to_string(timestamp) + "_" +
                    std::to_string(instance) + ".dat";

                TSPGenerator::saveToFile(filename, costs,
                    "Circuit board instance\nSize: " + category + "\nNodes: " + std::to_string(N));

                std::cout << "Generated instance: " << filename << " (nodes: " << N << ")\n";

                std::cout << "\nBoard Manufacturing Specifications:\n"
                    << "- Dimensions: " << width << "x" << height << " mm\n"
                    << "- Components: " << components << "\n"
                    << "- Total holes: " << N << "\n"
                    << "- Min hole spacing: " << TSPGenerator::MIN_HOLE_SPACING << " mm\n"
                    << "- Edge clearance: " << TSPGenerator::EDGE_MARGIN << " mm\n\n";

                DECL_ENV(env);
                DECL_PROB(env, lp);

                // Set time limit based on problem size
                double time_limit = N <= 20 ? 10.0 :     // 10 seconds for small
                    N <= 35 ? 60.0 :     // 1 minute for medium
                    300.0;               // 5 minutes for large

                CHECKED_CPX_CALL(CPXsetdblparam, env, CPX_PARAM_TILIM, time_limit);

                auto model_start = std::chrono::high_resolution_clock::now();
                TSPModel model;
                model.createModel(env, lp, N, costs);
                auto solve_start = std::chrono::high_resolution_clock::now();

                double objval;
                std::vector<int> tour;
                bool solved = model.solve(env, lp, objval, tour);
                auto end = std::chrono::high_resolution_clock::now();

                if (solved) {
                    double setup_time = std::chrono::duration<double>(solve_start - model_start).count();
                    double solve_time = std::chrono::duration<double>(end - solve_start).count();
                    double total_time = setup_time + solve_time;

                    int solstat = CPXgetstat(env, lp);
                    bool optimal = (solstat == CPXMIP_OPTIMAL);

                    double best_bound;
                    CPXgetbestobjval(env, lp, &best_bound);
                    double gap = ((objval - best_bound) / objval) * 100.0;

                    std::cout << "Performance Metrics:\n"
                        << "- Model setup time: " << setup_time << " seconds\n"
                        << "- Solution time: " << solve_time << " seconds\n"
                        << "- Total time: " << total_time << " seconds\n"
                        << "- Solution status: " << (optimal ? "Optimal" : "Not optimal") << "\n"
                        << "- Optimality gap: " << std::fixed << std::setprecision(2)
                        << gap << "%\n\n";

                    std::cout << "Solution Quality:\n"
                        << "- Total drilling path length: " << objval << " mm\n"
                        << "- Average distance between holes: " << objval / N << " mm\n\n";

                    std::cout << "Drilling sequence: ";
                    for (int node : tour) {
                        std::cout << node << " -> ";
                    }
                    std::cout << "0\n\n";
                }

                CPXfreeprob(env, &lp);
                CPXcloseCPLEX(&env);
            }
        }

        return 0;
    }
    catch (std::exception& e) {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
}