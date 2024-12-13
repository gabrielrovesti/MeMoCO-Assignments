#include "parameter_calibration.h"
#include "TSPSolver.h"
#include <chrono>
#include <numeric>
#include <tuple>
#include <cmath>
#include <algorithm>
#include <iostream>

ParameterCalibration::Parameters ParameterCalibration::calibrateParameters(
    const std::vector<std::tuple<int, int, int>>& board_configs) {

    Parameters best_params;
    std::vector<TSP> training_instances;

    // Generate training instances
    for (const auto& config : board_configs) {
        for (int i = 0; i < 5; i++) {
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
    double best_small_quality = std::numeric_limits<double>::max();
    double best_medium_quality = std::numeric_limits<double>::max();
    double best_large_quality = std::numeric_limits<double>::max();

    for (int tenure : tenure_values) {
        for (int iter_mult : iteration_multipliers) {
            CalibrationResult result = testParameterCombination(
                training_instances, tenure, iter_mult);

            // Update parameters based on instance size
            if (result.avg_solution_quality < best_small_quality && result.tenure <= 7) {
                best_small_quality = result.avg_solution_quality;
                best_params.small_tenure = result.tenure;
                best_params.small_iterations = result.iterations;
            }
            if (result.avg_solution_quality < best_medium_quality && result.tenure <= 9) {
                best_medium_quality = result.avg_solution_quality;
                best_params.medium_tenure = result.tenure;
                best_params.medium_iterations = result.iterations;
            }
            if (result.avg_solution_quality < best_large_quality) {
                best_large_quality = result.avg_solution_quality;
                best_params.large_tenure = result.tenure;
                best_params.large_iterations = result.iterations;
            }
        }
    }

    return best_params;
}

ParameterCalibration::CalibrationResult ParameterCalibration::testParameterCombination(
    const std::vector<TSP>& instances, int tenure, int iteration_multiplier) {

    CalibrationResult result(tenure, iteration_multiplier);
    std::vector<double> qualities;

    for (const auto& instance : instances) {
        TSPSolver solver;
        TSPSolution initial(instance);
        TSPSolution final(instance);

        solver.initRnd(initial);
        solver.setTabuTenure(tenure);
        solver.setMaxIterations(instance.n * iteration_multiplier);

        auto start = std::chrono::high_resolution_clock::now();

        if (solver.solveWithTabuSearch(instance, initial, final)) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                end - start).count();

            double quality = solver.evaluate(final, instance);
            qualities.push_back(quality);
            result.avg_time_ms += duration;
        }
    }

    if (!qualities.empty()) {
        result.avg_solution_quality = std::accumulate(
            qualities.begin(), qualities.end(), 0.0) / qualities.size();
        result.avg_time_ms /= qualities.size();

        double variance = 0.0;
        for (double q : qualities) {
            variance += std::pow(q - result.avg_solution_quality, 2);
        }
        result.std_dev_quality = std::sqrt(variance / qualities.size());
    }

    return result;
}