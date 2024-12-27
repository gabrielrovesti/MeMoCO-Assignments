#include "parameter_calibration.h"
#include "TSPSolver.h"
#include "data_generator.h"
#include <chrono>
#include <numeric>
#include <cmath>
#include <iostream>

ParameterCalibration::Parameters ParameterCalibration::calibrateParameters(
    const std::vector<std::tuple<int, int, int>>& board_configs) {

    Parameters best_params;
    std::vector<TSP> training_instances;

    // Generate training instances
    for (const auto& config : board_configs) {
        for (int i = 0; i < 5; i++) {  // 5 instances per configuration
            auto costs = TSPGenerator::generateCircuitBoard(
                std::get<0>(config),
                std::get<1>(config),
                std::get<2>(config)
            );

            TSP instance;
            instance.n = costs.size();
            instance.cost = costs;
            instance.infinite = 1e10;
            training_instances.push_back(instance);
        }
    }

    // Test parameter combinations
    double best_quality = std::numeric_limits<double>::infinity();

    // Print table header
    std::cout << "\n" << std::left << std::setw(10) << "Tenure"
        << std::setw(15) << "Iterations"
        << std::setw(15) << "Avg. Quality"
        << std::setw(15) << "Std. Dev."
        << std::setw(15) << "Avg. Time (ms)" << "\n";
    std::cout << std::setfill('-') << std::setw(70) << "" << std::setfill(' ') << "\n";

    for (int tenure : tenure_values) {
        for (int iter_mult : iteration_multipliers) {
            CalibrationResult result = testParameterCombination(
                training_instances, tenure, iter_mult);

            int iterations = iter_mult * training_instances[0].n;

            // Print calibration results in a tabular format
            std::cout << std::left << std::setw(10) << tenure
                << std::setw(15) << iterations
                << std::fixed << std::setprecision(3)
                << std::setw(15) << result.avg_solution_quality
                << std::setw(15) << result.std_dev_quality
                << std::setw(15) << result.avg_time_ms << "\n";

            // Update parameters based on instance size
            if (result.avg_solution_quality < best_quality) {
                best_quality = result.avg_solution_quality;

                for (const auto& instance : training_instances) {
                    if (instance.n <= 20) {
                        best_params.small_tenure = tenure;
                        best_params.small_iterations = iter_mult * instance.n;
                    }
                    else if (instance.n <= 35) {
                        best_params.medium_tenure = tenure;
                        best_params.medium_iterations = iter_mult * instance.n;
                    }
                    else {
                        best_params.large_tenure = tenure;
                        best_params.large_iterations = iter_mult * instance.n;
                    }
                }
            }
        }
    }

    // Print summary of best parameters
    std::cout << "\nBest parameters found:\n";
    std::cout << "Small instances - Tenure: " << best_params.small_tenure
        << ", Iterations: " << best_params.small_iterations << "\n";
    std::cout << "Medium instances - Tenure: " << best_params.medium_tenure
        << ", Iterations: " << best_params.medium_iterations << "\n";
    std::cout << "Large instances - Tenure: " << best_params.large_tenure
        << ", Iterations: " << best_params.large_iterations << "\n";

    return best_params;
}

ParameterCalibration::CalibrationResult ParameterCalibration::testParameterCombination(
    const std::vector<TSP>& instances,
    int tenure,
    int iteration_multiplier) {

    CalibrationResult result{ tenure, iteration_multiplier, 0.0, 0.0, 0.0 };
    std::vector<double> qualities;
    double total_time_ms = 0.0;

    for (const auto& instance : instances) {
        TSPSolver solver;
        TSPSolution initial(instance);
        TSPSolution final(instance);

        solver.initRnd(initial);
        solver.setTabuTenure(tenure);
        solver.setMaxIterations(instance.n * iteration_multiplier);

        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::pair<double, double>> points;

        if (solver.solveWithTabuSearch(instance, initial, final, points)) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                end - start).count();

            double quality = solver.evaluate(final, instance);
            qualities.push_back(quality);
            total_time_ms += duration;
        }
    }

    if (!qualities.empty()) {
        result.avg_solution_quality = std::accumulate(
            qualities.begin(), qualities.end(), 0.0) / qualities.size();
        result.avg_time_ms = total_time_ms / instances.size();

        double variance = 0.0;
        for (double q : qualities) {
            variance += std::pow(q - result.avg_solution_quality, 2);
        }
        result.std_dev_quality = std::sqrt(variance / qualities.size());
    }

    return result;
}