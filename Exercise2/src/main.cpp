#include <stdexcept>
#include <chrono>
#include <windows.h>
#include <iomanip>
#include <algorithm>
#include <direct.h>
#include "TSPSolver.h"
#include "data_generator.h"
#include "parameter_calibration.h"
#include <filesystem>

int status;
char errmsg[255];

bool createDirectory(const std::string& path) {
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
}

void generateInstanceSet(const std::vector<std::tuple<int, int, int>>& board_configs,
    int instances_per_size) {
    if (!createDirectory("data")) {
        throw std::runtime_error("Failed to create data directory");
    }
    createDirectory("data/small");
    createDirectory("data/medium");
    createDirectory("data/large");

    for (const auto& config : board_configs) {
        int width = std::get<0>(config);
        int height = std::get<1>(config);
        int components = std::get<2>(config);

        for (int i = 0; i < instances_per_size; i++) {
            auto costs = TSPGenerator::generateCircuitBoard(width, height, components);
            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                now.time_since_epoch()).count();

            std::string category = costs.size() <= 20 ? "small" :
                costs.size() <= 35 ? "medium" : "large";

            std::string filename = "data/" + category + "/board_" +
                std::to_string(width) + "x" +
                std::to_string(height) + "_" +
                std::to_string(timestamp) + "_" +
                std::to_string(i) + ".dat";

            try {
                TSPGenerator::saveToFile(filename, costs,
                    "Circuit board instance\n"
                    "Size: " + category + "\n"
                    "Nodes: " + std::to_string(costs.size()));

                std::cout << "Generated instance: " << filename
                    << " (nodes: " << costs.size() << ")\n";
            }
            catch (const std::exception& e) {
                std::cout << "Error saving instance: " << e.what() << "\n";
            }
        }
    }
}

int main(int argc, char const* argv[]) {
    try {
        // Board configurations for testing
        std::vector<std::tuple<int, int, int>> board_configs = {
            {50, 50, 2},    // Small boards (~10-15 holes)
            {75, 75, 3},    // Medium-small boards (~15-20 holes)
            {100, 100, 3},  // Medium boards (~25-30 holes)
            {125, 125, 4},  // Medium-large boards (~35-40 holes)
            {150, 150, 5},  // Large boards (~45-50 holes)
        };

        // Phase 1: Instance Generation
        std::cout << "Phase 1: Generating Training Instances\n";
        const int INSTANCES_PER_SIZE = 10;
        generateInstanceSet(board_configs, INSTANCES_PER_SIZE);

        // Phase 2: Parameter Calibration
        std::cout << "\nPhase 2: Parameter Calibration\n";
        ParameterCalibration calibrator;
        ParameterCalibration::Parameters params = calibrator.calibrateParameters(board_configs);

        std::cout << "\nCalibration complete. Optimal parameters found:\n";
        std::cout << "Small instances - Tenure: " << params.small_tenure
            << ", Iterations: " << params.small_iterations << "\n";
        std::cout << "Medium instances - Tenure: " << params.medium_tenure
            << ", Iterations: " << params.medium_iterations << "\n";
        std::cout << "Large instances - Tenure: " << params.large_tenure
            << ", Iterations: " << params.large_iterations << "\n";

        // Phase 3: Solution Visualization
        std::cout << "\nPhase 3: Solution Visualization\n";
        for (const auto& category : { "small", "medium", "large" }) {
            std::cout << "Processing " << category << " instances...\n";

            std::string data_path = "data/" + std::string(category);
            std::wstring search_path = std::wstring(data_path.begin(), data_path.end()) + L"\\*.dat";

            WIN32_FIND_DATAW file_data;
            HANDLE dir = FindFirstFileW(search_path.c_str(), &file_data);

            if (dir == INVALID_HANDLE_VALUE) {
                std::cout << "No instances found in " << data_path << "\n";
                continue;
            }

            do {
                // Convert wide string to regular string for comparison
                std::wstring ws(file_data.cFileName);
                std::string filename(ws.begin(), ws.end());

                if (filename == "." || filename == "..") {
                    continue;
                }

                std::string instance_file = data_path + "\\" + filename;
                std::cout << "Processing instance: " << filename << "\n";

                try {
                    auto costs = TSPGenerator::loadFromFile(instance_file);
                    TSP tsp;
                    tsp.n = costs.size();
                    tsp.cost = costs;

                    // Rest of the solver configuration and execution remains the same
                    TSPSolution initialSol(tsp);
                    TSPSolution bestSol(tsp);
                    TSPSolver solver;

                    if (tsp.n <= 20) {
                        solver.setTabuTenure(params.small_tenure);
                        solver.setMaxIterations(params.small_iterations);
                    }
                    else if (tsp.n <= 35) {
                        solver.setTabuTenure(params.medium_tenure);
                        solver.setMaxIterations(params.medium_iterations);
                    }
                    else {
                        solver.setTabuTenure(params.large_tenure);
                        solver.setMaxIterations(params.large_iterations);
                    }

                    solver.initRnd(initialSol);
                    solver.solveWithTabuSearch(tsp, initialSol, bestSol);
                }
                catch (const std::exception& e) {
                    std::cout << "Error processing instance " << filename
                        << ": " << e.what() << "\n";
                    continue;
                }

            } while (FindNextFileW(dir, &file_data));

            FindClose(dir);
        }

        std::cout << "\nGenerating Performance Analysis...\n";
        std::string command = "python plot.py data";  // Pass data directory for analysis
        int result = system(command.c_str());
        if (result == 0) {
            std::cout << "Performance analysis complete. Check performance_analysis.png\n";
        }
        else {
            std::cout << "Warning: Performance analysis generation failed\n";
        }

        return 0;
    }
    catch (std::exception& e) {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
}