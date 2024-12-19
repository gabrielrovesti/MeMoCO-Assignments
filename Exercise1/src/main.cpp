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