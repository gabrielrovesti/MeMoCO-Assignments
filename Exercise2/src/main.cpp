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

struct TestResults {
    double initial_cost;
    double final_cost;
    double improvement_percentage;
    double execution_time;
    double avg_time;
    double best_cost;
    double worst_cost;
};

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

TestResults runBenchmark(const TSP& tsp, TSPSolver& solver, int num_runs = 10) {
    std::vector<double> solution_costs;
    std::vector<double> run_times;
    double initial_cost = 0.0;

    for (int run = 0; run < num_runs; run++) {
        TSPSolution initial(tsp);
        TSPSolution best(tsp);
        solver.initRnd(initial);

        if (run == 0) {
            initial_cost = solver.evaluate(initial, tsp);
        }

        auto start = std::chrono::high_resolution_clock::now();
        solver.solveWithTabuSearch(tsp, initial, best, {});
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

    TestResults results;
    results.initial_cost = initial_cost;
    results.final_cost = min_cost;
    results.improvement_percentage = ((initial_cost - min_cost) / initial_cost) * 100.0;
    results.execution_time = avg_time;
    results.avg_time = avg_time;
    results.best_cost = min_cost;
    results.worst_cost = max_cost;

    return results;
}

void solveAndVisualize(const TSP& tsp, const std::vector<std::pair<double, double>>& points,
    const ParameterCalibration::Parameters& params, const std::string& output_prefix) {

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

    TSPSolution initialSol(tsp);
    solver.initRnd(initialSol);
    double initialCost = solver.evaluate(initialSol, tsp);

    TSPSolution bestSol(tsp);
    auto start = std::chrono::high_resolution_clock::now();
    solver.solveWithTabuSearch(tsp, initialSol, bestSol, points);
    auto end = std::chrono::high_resolution_clock::now();

    double finalCost = solver.evaluate(bestSol, tsp);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    BoardVisualizer::generateSVG(points, initialSol.sequence,
        output_prefix + "_initial.svg", true, 0, initialCost);

    BoardVisualizer::generateSVG(points, bestSol.sequence,
        output_prefix + "_final.svg", true, -1, finalCost);

    BoardVisualizer::generateComparisonSVG(points, initialSol.sequence, bestSol.sequence,
        output_prefix + "_comparison.svg", initialCost, finalCost);

    double improvement = ((initialCost - finalCost) / initialCost) * 100.0;
    std::cout << "Results for " << output_prefix << ":\n"
        << "  Initial cost: " << initialCost << "\n"
        << "  Final cost: " << finalCost << "\n"
        << "  Improvement: " << std::fixed << std::setprecision(2)
        << improvement << "%\n"
        << "  Time: " << duration.count() << "ms\n\n";
}

void analyzeResults(const std::vector<TestResults>& results, std::ofstream& log_file) {
    double total_improvement = 0.0;
    double total_time = 0.0;
    double best_improvement = 0.0;
    double worst_improvement = (std::numeric_limits<double>::max)();

    for (const auto& result : results) {
        total_improvement += result.improvement_percentage;
        total_time += result.execution_time;
        // Replace std::max/min with direct comparisons
        if (result.improvement_percentage > best_improvement) {
            best_improvement = result.improvement_percentage;
        }
        if (result.improvement_percentage < worst_improvement) {
            worst_improvement = result.improvement_percentage;
        }
    }

    double avg_improvement = total_improvement / results.size();
    double avg_time = total_time / results.size();

    log_file << "\nFinal Analysis:\n"
        << "Average Improvement: " << std::fixed << std::setprecision(2)
        << avg_improvement << "%\n"
        << "Best Improvement: " << best_improvement << "%\n"
        << "Worst Improvement: " << worst_improvement << "%\n"
        << "Average Execution Time: " << avg_time << "ms\n";
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

        createDirectory("data");
        createDirectory("visualizations");
        createDirectory("results");

        std::cout << "Phase 1: Generating Training Instances\n"
            << "=====================================\n";
        generateInstanceSet(board_configs, 10);

        std::cout << "\nPhase 2: Parameter Calibration\n"
            << "=============================\n";
        ParameterCalibration calibrator;
        auto params = calibrator.calibrateParameters(board_configs);

        std::ofstream calibration_log("results/calibration_results.txt");
        calibration_log << "Calibration Results:\n"
            << "Small instances - Tenure: " << params.small_tenure
            << ", Iterations: " << params.small_iterations << "\n"
            << "Medium instances - Tenure: " << params.medium_tenure
            << ", Iterations: " << params.medium_iterations << "\n"
            << "Large instances - Tenure: " << params.large_tenure
            << ", Iterations: " << params.large_iterations << "\n";

        std::cout << "\nPhase 3: Testing and Visualization\n"
            << "================================\n";
        std::ofstream results_log("results/benchmark_results.txt");
        std::vector<TestResults> all_results;

        for (const auto& config : board_configs) {
            int width = std::get<0>(config);
            int height = std::get<1>(config);
            int components = std::get<2>(config);

            auto costs = TSPGenerator::generateCircuitBoard(width, height, components);
            std::vector<std::pair<double, double>> points;
            for (const auto& p : TSPGenerator::getLastGeneratedPoints()) {
                points.push_back({ p.x, p.y });
            }

            TSP tsp;
            tsp.n = costs.size();
            tsp.cost = costs;
            tsp.infinite = std::numeric_limits<double>::infinity();

            std::string prefix = "visualizations/board_" +
                std::to_string(width) + "x" + std::to_string(height);

            std::cout << "\nTesting " << width << "x" << height
                << " board (" << tsp.n << " holes):\n";

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

            solveAndVisualize(tsp, points, params, prefix);
            TestResults bench_results = runBenchmark(tsp, solver);
            all_results.push_back(bench_results);

            results_log << "\nInstance " << width << "x" << height
                << " (" << tsp.n << " nodes):\n"
                << "Initial Cost: " << bench_results.initial_cost << "\n"
                << "Best Cost: " << bench_results.best_cost << "\n"
                << "Worst Cost: " << bench_results.worst_cost << "\n"
                << "Improvement: " << bench_results.improvement_percentage << "%\n"
                << "Average Time: " << bench_results.avg_time << "ms\n";
        }

        analyzeResults(all_results, results_log);

        results_log.close();
        calibration_log.close();

        return 0;
    }
    catch (std::exception& e) {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
}