#include "parameter_calibration.h"
#include "TSPSolver.h"
#include "data_generator.h"
#include <chrono>
#include <numeric>
#include <cmath>
#include <iostream>

void ParameterCalibration::calibrateParameters(
    const std::vector<std::tuple<int, int, int>>& board_configs) {

    std::vector<TSP> training_instances;

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

    std::cout << "\n=== Parameter Calibration Results ===\n";
    std::cout << "Testing " << tenure_values.size() * iteration_multipliers.size()
        << " parameter combinations\n";

    for (int tenure : tenure_values) {
        for (int iter_mult : iteration_multipliers) {
            CalibrationResult result = testParameterCombination(
                training_instances, tenure, iter_mult);

            std::cout << "\nTenure: " << tenure
                << ", Iteration Multiplier: " << iter_mult << "\n";
            std::cout << "Average solution quality: " << result.avg_solution_quality << "\n";
            std::cout << "Average time (ms): " << result.avg_time_ms << "\n";
            std::cout << "Quality std dev: " << result.std_dev_quality << "\n";
        }
    }
}

ParameterCalibration::CalibrationResult ParameterCalibration::testParameterCombination(
    const std::vector<TSP>& instances,
    int tenure,
    int iteration_multiplier) {

    CalibrationResult result{ tenure, iteration_multiplier, 0.0, 0.0, 0.0 };
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

    result.avg_solution_quality = std::accumulate(
        qualities.begin(), qualities.end(), 0.0) / qualities.size();
    result.avg_time_ms /= instances.size();

    double variance = 0.0;
    for (double q : qualities) {
        variance += std::pow(q - result.avg_solution_quality, 2);
    }
    result.std_dev_quality = std::sqrt(variance / qualities.size());

    return result;
}