#ifndef PARAMETERCALIBRATION_H
#define PARAMETERCALIBRATION_H

#include <vector>
#include <tuple>
#include "TSP.h"

class ParameterCalibration {
private:
    struct CalibrationResult {
        int tenure;
        int iterations;
        double avg_solution_quality;
        double avg_time_ms;
        double std_dev_quality;
    };

    // Parameters ranges to test
    const std::vector<int> tenure_values = { 5, 7, 9, 11, 13 };
    const std::vector<int> iteration_multipliers = { 10, 15, 20, 25, 30 };

    // Private member function
    CalibrationResult testParameterCombination(
        const std::vector<TSP>& instances,
        int tenure,
        int iteration_multiplier);

public:
    void calibrateParameters(const std::vector<std::tuple<int, int, int>>& board_configs);
};

#endif // PARAMETERCALIBRATION_H