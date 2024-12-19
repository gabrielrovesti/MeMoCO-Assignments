/**
* @file parameter_calibration.h
* @brief Parameter calibration system for the TSP solver
*
* This class handles automated parameter tuning for the Tabu Search solver.
* It performs systematic testing of different parameter combinations on training
* instances to determine optimal settings for different problem sizes.
*
* Features:
* - Separate parameter sets for small/medium/large instances
* - Tests multiple tenure/iteration combinations
* - Statistical analysis of solution quality
* - Automated quality/runtime tradeoff evaluation
*/

#ifndef PARAMETER_CALIBRATION_H
#define PARAMETER_CALIBRATION_H

#include <vector>
#include <tuple>
#include "TSP.h"

class ParameterCalibration {
public:
    struct Parameters {
        int small_tenure;
        int medium_tenure;
        int large_tenure;
        int small_iterations;
        int medium_iterations;
        int large_iterations;

        Parameters() :
            small_tenure(5), medium_tenure(7), large_tenure(9),
            small_iterations(100), medium_iterations(200), large_iterations(300) {}
    };

    struct CalibrationResult {
        int tenure;
        int iterations;
        double avg_solution_quality;
        double avg_time_ms;
        double std_dev_quality;

        CalibrationResult(int t = 0, int i = 0, double q = 0.0, double time = 0.0, double dev = 0.0)
            : tenure(t), iterations(i), avg_solution_quality(q), avg_time_ms(time), std_dev_quality(dev) {}
    };

    Parameters calibrateParameters(const std::vector<std::tuple<int, int, int>>& board_configs);

private:
    const std::vector<int> tenure_values = { 5, 7, 9, 11, 13 };
    const std::vector<int> iteration_multipliers = { 10, 15, 20, 25, 30 };

    CalibrationResult testParameterCombination(
        const std::vector<TSP>& instances,
        int tenure,
        int iteration_multiplier);
};

#endif