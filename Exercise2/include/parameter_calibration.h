#ifndef PARAMETER_CALIBRATION_H
#define PARAMETER_CALIBRATION_H

#include <vector>
#include <tuple>

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
            small_tenure(0), medium_tenure(0), large_tenure(0),
            small_iterations(0), medium_iterations(0), large_iterations(0) {}
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

    void calibrateParameters(const std::vector<std::tuple<int, int, int>>& board_configs);

private:
    const std::vector<int> tenure_values = { 5, 7, 9, 11, 13 };
    const std::vector<int> iteration_multipliers = { 10, 15, 20, 25, 30 };

    CalibrationResult testParameterCombination(
        const std::vector<TSP>& instances,
        int tenure,
        int iteration_multiplier);
};

#endif