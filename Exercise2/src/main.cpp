#include <stdexcept>
#include <chrono>
#include <windows.h>
#include <iomanip>
#include <algorithm>
#include <direct.h>
#include <filesystem>
#include <fstream>
#include <limits>
#include <numeric>
#include "TSPSolver.h"
#include "data_generator.h"
#include "parameter_calibration.h"
#include "visualization.h"

int status;
char errmsg[255];

bool createDirectory(const std::string& path) {
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
}

void generateInstanceSet(const std::vector<std::tuple<int, int, int>>& board_configs,
    int instances_per_size) {

    createDirectory("data");
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
                std::to_string(width) + "x" + std::to_string(height) + "_" +
                std::to_string(timestamp) + "_" + std::to_string(i) + ".dat";

            try {
                TSPGenerator::saveToFile(filename, costs,
                    "Circuit board instance\nSize: " + category + "\nNodes: " +
                    std::to_string(costs.size()));

                std::cout << "Generated instance: " << filename
                    << " (nodes: " << costs.size() << ")\n";
            }
            catch (const std::exception& e) {
                std::cout << "Error saving instance: " << e.what() << "\n";
            }
        }
    }
}

void calculateStats(const std::vector<double>& values, double& avg, double& min, double& max) {
    if (values.empty()) return;

    min = values[0];
    max = values[0];
    double sum = 0.0;

    for (const auto& val : values) {
        sum += val;
        if (val < min) min = val;
        if (val > max) max = val;
    }

    avg = sum / values.size();
}

void runBenchmark(const TSP& tsp, TSPSolver& solver, int num_runs = 10) {
    std::vector<double> solution_costs;
    std::vector<double> run_times;

    for (int run = 0; run < num_runs; run++) {
        TSPSolution initial(tsp);
        TSPSolution best(tsp);
        solver.initRnd(initial);

        auto start = std::chrono::high_resolution_clock::now();
        solver.solveWithTabuSearch(tsp, initial, best, {});  // Empty points for benchmark runs
        auto end = std::chrono::high_resolution_clock::now();

        double cost = solver.evaluate(best, tsp);
        double time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        solution_costs.push_back(cost);
        run_times.push_back(time);
    }

    double avg_cost, min_cost, max_cost, avg_time, min_time, max_time;
    calculateStats(solution_costs, avg_cost, min_cost, max_cost);
    calculateStats(run_times, avg_time, min_time, max_time);

    std::cout << "Benchmark Results (" << num_runs << " runs):\n"
        << "  Average Cost: " << std::fixed << std::setprecision(2) << avg_cost << "\n"
        << "  Best Cost: " << min_cost << "\n"
        << "  Worst Cost: " << max_cost << "\n"
        << "  Average Time: " << avg_time << "ms\n";
}

void solveAndVisualize(const TSP& tsp, const std::vector<std::pair<double, double>>& points,
    const ParameterCalibration::Parameters& params, const std::string& output_prefix) {

    TSPSolver solver;

    // Configure solver based on problem size
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

    // Create and save initial solution
    TSPSolution initialSol(tsp);
    solver.initRnd(initialSol);
    double initialCost = solver.evaluate(initialSol, tsp);

    // Solve and measure performance
    TSPSolution bestSol(tsp);
    auto start = std::chrono::high_resolution_clock::now();
    solver.solveWithTabuSearch(tsp, initialSol, bestSol, points);
    auto end = std::chrono::high_resolution_clock::now();

    double finalCost = solver.evaluate(bestSol, tsp);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Generate visualizations
    BoardVisualizer::generateSVG(points, initialSol.sequence,
        output_prefix + "_initial.svg", true, 0, initialCost);

    BoardVisualizer::generateSVG(points, bestSol.sequence,
        output_prefix + "_final.svg", true, -1, finalCost);

    BoardVisualizer::generateComparisonSVG(points, initialSol.sequence, bestSol.sequence,
        output_prefix + "_comparison.svg", initialCost, finalCost);

    // Report results
    double improvement = ((initialCost - finalCost) / initialCost) * 100.0;
    std::cout << "Results for " << output_prefix << ":\n"
        << "  Initial cost: " << initialCost << "\n"
        << "  Final cost: " << finalCost << "\n"
        << "  Improvement: " << std::fixed << std::setprecision(2)
        << improvement << "%\n"
        << "  Time: " << duration.count() << "ms\n\n";
}

int main(int argc, char const* argv[]) {
    try {
        std::vector<std::tuple<int, int, int>> board_configs = {
            {50, 50, 2},    // Small boards
            {75, 75, 3},    // Medium-small boards
            {100, 100, 3},  // Medium boards
            {125, 125, 4},  // Medium-large boards
            {150, 150, 5}   // Large boards
        };

        // Create output directories
        createDirectory("data");
        createDirectory("visualizations");

        std::cout << "Phase 1: Generating Training Instances\n"
            << "=====================================\n";
        generateInstanceSet(board_configs, 10);

        std::cout << "\nPhase 2: Parameter Calibration\n"
            << "=============================\n";
        ParameterCalibration calibrator;
        auto params = calibrator.calibrateParameters(board_configs);

        std::cout << "\nPhase 3: Testing and Visualization\n"
            << "================================\n";

        for (const auto& config : board_configs) {
            int width = std::get<0>(config);
            int height = std::get<1>(config);
            int components = std::get<2>(config);

            // Generate test instance
            auto costs = TSPGenerator::generateCircuitBoard(width, height, components);
            std::vector<std::pair<double, double>> points;
            for (const auto& p : TSPGenerator::getLastGeneratedPoints()) {
                points.push_back({ p.x, p.y });
            }

            // Setup problem instance
            TSP tsp;
            tsp.n = costs.size();
            tsp.cost = costs;
            tsp.infinite = std::numeric_limits<double>::infinity();

            std::string prefix = "visualizations/board_" +
                std::to_string(width) + "x" + std::to_string(height);

            std::cout << "\nTesting " << width << "x" << height
                << " board (" << tsp.n << " holes):\n";

            // Solve with visualization
            solveAndVisualize(tsp, points, params, prefix);

            // Run benchmarks
            std::cout << "Running benchmarks...\n";
            TSPSolver benchmark_solver;
            if (tsp.n <= 20) {
                benchmark_solver.setTabuTenure(params.small_tenure);
                benchmark_solver.setMaxIterations(params.small_iterations);
            }
            else if (tsp.n <= 35) {
                benchmark_solver.setTabuTenure(params.medium_tenure);
                benchmark_solver.setMaxIterations(params.medium_iterations);
            }
            else {
                benchmark_solver.setTabuTenure(params.large_tenure);
                benchmark_solver.setMaxIterations(params.large_iterations);
            }
            runBenchmark(tsp, benchmark_solver);
        }

        return 0;
    }
    catch (std::exception& e) {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
}