#include <stdexcept>
#include <chrono>
#include <windows.h>
#include <iomanip>
#include <algorithm>
#include <direct.h>
#include "TSPSolver.h"
#include "data_generator.h"
#include "parameter_calibration.h"
#include "visualization.h"
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

        std::cout << "\nGenerating Visualizations...\n";
        if (!std::filesystem::exists("visualizations")) {
            std::filesystem::create_directory("visualizations");
        }

        for (const auto& config : board_configs) {
            int width = std::get<0>(config);
            int height = std::get<1>(config);
            int components = std::get<2>(config);

            // Generate instance and solve
            auto costs = TSPGenerator::generateCircuitBoard(width, height, components);
            TSP tsp;
            tsp.n = costs.size();
            tsp.cost = costs;

            std::vector<TSPGenerator::Point> points = TSPGenerator::getLastGeneratedPoints();
            std::vector<std::pair<double, double>> viz_points;
            for (const auto& p : points) {
                viz_points.push_back({ p.x, p.y });
            }

            TSPSolution initialSol(tsp);
            TSPSolution bestSol(tsp);
            TSPSolver solver;

            // Configure solver
            int maxIter;
            if (tsp.n <= 20) {
                solver.setTabuTenure(params.small_tenure);
                maxIter = params.small_iterations;
            }
            else if (tsp.n <= 35) {
                solver.setTabuTenure(params.medium_tenure);
                maxIter = params.medium_iterations;
            }
            else {
                solver.setTabuTenure(params.large_tenure);
                maxIter = params.large_iterations;
            }
            solver.setMaxIterations(maxIter);

            solver.initRnd(initialSol);

            // Generate base filename
            std::string baseName = "visualizations/board_" +
                std::to_string(width) + "x" + std::to_string(height) + "_n" +
                std::to_string(tsp.n);

            // Save initial board layout
            BoardVisualizer::generateSVG(viz_points, {},
                baseName + "_layout.svg", false);
            std::cout << "Saved board layout: " << baseName + "_layout.svg\n";

            // Solve and save solution
            if (solver.solveWithTabuSearch(tsp, initialSol, bestSol, viz_points)) {
                BoardVisualizer::generateSVG(viz_points, bestSol.sequence,
                    baseName + "_solution.svg", true);
                std::cout << "Saved solution visualization: " << baseName + "_solution.svg\n";

                double finalCost = solver.evaluate(bestSol, tsp);
                std::cout << "Board " << width << "x" << height << " (" << tsp.n
                    << " holes) - Final cost: " << finalCost << "\n\n";
            }
		}
		return 0;

    }
    catch (std::exception& e) {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
}