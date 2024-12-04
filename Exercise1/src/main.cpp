/**
* @file main.cpp
* @brief Optimized implementation of flow-based TSP for drilling problem
*
* This implementation uses the simplified flow-based formulation from
* "The travelling salesman problem and related problems" (Gavish & Graves, 1978).
* The formulation removes redundant constraints and variables to origin.
*
* Mathematical Model:
* Min Σ(i,j∈A) cij * yij                                 (9) - Minimize total travel cost
* s.t.
* Σ(i:(i,k)∈A) xik - Σ(j:(k,j),j≠0) xkj = 1            (10) - Flow conservation excluding origin
* Σ(j:(i,j)∈A) yij = 1                                  (11) - One outgoing arc per node
* Σ(i:(i,j)∈A) yij = 1                                  (12) - One incoming arc per node
* xij ≤ (|N|-1)yij                     ∀(i,j)∈A, j≠0    (13) - Linking x and y variables
* xij ∈ R+                             ∀(i,j)∈A, j≠0    (14) - Flow variables non-negative
* yij ∈ {0,1}                          ∀(i,j)∈A         (15) - Binary path variables
*
* Variables:
* xij: Amount of flow on arc (i,j), j≠0 (no flow to origin)
* yij: 1 if arc (i,j) is in the tour, 0 otherwise
*
* Parameters:
* N: Number of nodes (including depot)
* cij: Cost/distance between nodes i and j
*
* Key Optimizations:
* 1. Removed redundant origin flow constraint
* 2. Eliminated variables for flow to origin (xi0)
* 3. Simplified flow conservation constraint
* 4. Reduced number of linking constraints
*/

#include <iostream>
#include <iomanip>
#include <cpxmacro.h>
#include <model.h>
#include <data_generator.h>
#include <chrono>
#include <tuple>

// For Windows directory operations (std::filesystem was the first idea - was not able to make it work)
// so I found a workaround with this one instead
#include <direct.h> 

// Error status and message buffer
int status;
char errmsg[BUF_SIZE];

bool directoryExists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

bool createDirectoryIfNeeded(const std::string& path) {
    if (!directoryExists(path)) {
#ifdef _WIN32
        return _mkdir(path.c_str()) == 0;
#else
        return mkdir(path.c_str(), 0777) == 0;
#endif
    }
    return true;
}

std::string getSizeCategory(int N) {
    if (N <= 20) return "small";
    if (N <= 35) return "medium";
    return "large";
}

int main(int argc, char const* argv[]) {
    try {
        // Test different board configurations
        std::vector<std::tuple<int, int, int>> board_configs = {
            {50, 50, 2},     // Small board (~10-15 holes)
            {100, 100, 3},   // Medium board (~25-30 holes)
            {150, 150, 5},   // Large board (~45-50 holes)
            {200, 200, 8}    // Extra large board (~80-85 holes)
        };

        for (const auto& config : board_configs) {
            int width = std::get<0>(config);
            int height = std::get<1>(config);
            int components = std::get<2>(config);

            std::cout << "\n=== Testing circuit board " << width << "x" << height
                << " with " << components << " components ===\n";

            // Generate circuit board instance
            auto costs = TSPGenerator::generateCircuitBoard(width, height, components);
            int N = costs.size();

            // Determine correct size category based on number of holes
            std::string folder = getSizeCategory(N);

            // Ensure base data directory exists
            if (!createDirectoryIfNeeded("data")) {
                std::cerr << "Failed to create data directory" << std::endl;
                continue;
            }

            // Ensure category subdirectory exists
            std::string directory = "data/" + folder;
            if (!createDirectoryIfNeeded(directory)) {
                std::cerr << "Failed to create category directory: " << directory << std::endl;
                continue;
            }

            // Create unique filename with timestamp
            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                now.time_since_epoch()).count();

            std::string filename = directory + "/board_" +
                std::to_string(width) + "x" +
                std::to_string(height) + "_" +
                std::to_string(timestamp) + ".txt";

            // Save metadata about the board configuration
            std::string metadata = "Circuit board instance\n"
                "Width: " + std::to_string(width) + "\n"
                "Height: " + std::to_string(height) + "\n"
                "Components: " + std::to_string(components) + "\n"
                "Total holes: " + std::to_string(N) + "\n"
                "Size category: " + folder;

            // Save the instance with proper error handling
            try {
                TSPGenerator::saveToFile(filename, costs, metadata);
                std::cout << "Instance saved to: " << filename << "\n";
            }
            catch (const std::exception& e) {
                std::cerr << "Error saving instance: " << e.what() << "\n";
            }

            try {
                DECL_ENV(env);
                DECL_PROB(env, lp);

                // Set time limit based on problem size
                double time_limit = N <= 20 ? 10.0 :     // 10 seconds for small
                    N <= 35 ? 60.0 :     // 1 minute for medium
                    300.0;               // 5 minutes for large

                CHECKED_CPX_CALL(CPXsetdblparam, env, CPX_PARAM_TILIM, time_limit);

                // Print detailed board analysis
                std::cout << "\nBoard Manufacturing Specifications:\n";
                std::cout << "- Dimensions: " << width << "x" << height << " mm\n";
                std::cout << "- Components: " << components << "\n";
                std::cout << "- Total holes: " << N << "\n";
                std::cout << "- Min hole spacing: " << TSPGenerator::MIN_HOLE_SPACING << " mm\n";
                std::cout << "- Edge clearance: " << TSPGenerator::EDGE_MARGIN << " mm\n\n";

                // Create and solve model
                auto model_start = std::chrono::high_resolution_clock::now();
                TSPModel model;
                model.createModel(env, lp, N, costs);
                auto solve_start = std::chrono::high_resolution_clock::now();

                double objval;
                std::vector<int> tour;

                if (model.solve(env, lp, objval, tour)) {
                    auto end = std::chrono::high_resolution_clock::now();

                    double setup_time = std::chrono::duration<double>(solve_start - model_start).count();
                    double solve_time = std::chrono::duration<double>(end - solve_start).count();
                    double total_time = setup_time + solve_time;

                    std::cout << "Performance Metrics:\n";
                    std::cout << "- Model setup time: " << setup_time << " seconds\n";
                    std::cout << "- Solution time: " << solve_time << " seconds\n";
                    std::cout << "- Total time: " << total_time << " seconds\n\n";

                    std::cout << "Solution Quality:\n";
                    std::cout << "- Total drilling path length: " << objval << " mm\n";
                    std::cout << "- Average distance between holes: " << objval / N << " mm\n";

                    double gap;
                    if (CPXgetmiprelgap(env, lp, &gap) == 0) {
                        std::cout << "- Optimality gap: " << gap * 100 << "%\n";
                    }

                    std::cout << "\nDrilling sequence: ";
                    for (int node : tour) {
                        std::cout << node << " -> ";
                    }
                    std::cout << "0\n";
                }

                CPXfreeprob(env, &lp);
                CPXcloseCPLEX(&env);
            }
            catch (std::exception& e) {
                std::cout << "Error solving instance: " << e.what() << "\n";
            }
        }

        return 0;
    }
    catch (std::exception& e) {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
}