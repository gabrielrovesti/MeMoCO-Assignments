#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <string>
#include <cmath>
#include <filesystem>

namespace {
    double best_recorded_cost = std::numeric_limits<double>::infinity();
}

class BoardVisualizer {
private:
    static double& getBestRecordedCost() { return best_recorded_cost; }

public:
    static void generateSVG(const std::vector<std::pair<double, double>>& points,
        const std::vector<int>& tour,
        const std::string& filename,
        bool showPath = true,
        int iteration = -1,
        double currentCost = -1.0) {

        try {
            std::filesystem::path filePath(filename);
            std::filesystem::path dir = filePath.parent_path();

            if (!dir.empty() && !std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);
            }

            std::ofstream file(filename);
            if (!file.is_open()) return;

            // Calculate bounds
            double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
            for (size_t i = 0; i < points.size(); ++i) {
                if (points[i].first < minX) minX = points[i].first;
                if (points[i].second < minY) minY = points[i].second;
                if (points[i].first > maxX) maxX = points[i].first;
                if (points[i].second > maxY) maxY = points[i].second;
            }

            // Add margins
            double margin = 10.0;
            minX -= margin;
            minY -= margin;
            maxX += margin;
            maxY += margin;

            file << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
            file << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                << "viewBox=\"" << minX << " " << minY << " "
                << (maxX - minX) << " " << (maxY - minY) << "\">\n";

            // Add white background
            file << "<rect x=\"" << minX << "\" y=\"" << minY
                << "\" width=\"" << (maxX - minX) << "\" height=\"" << (maxY - minY)
                << "\" fill=\"white\"/>\n";

            // Add iteration and cost info
            if (iteration >= 0) {
                file << "<text x=\"" << minX + 5 << "\" y=\"" << minY + 15
                    << "\" font-family=\"Arial\" font-size=\"10\" fill=\"black\">"
                    << "Iteration: " << iteration;
                if (currentCost >= 0) {
                    file << " | Cost: " << std::fixed << std::setprecision(2) << currentCost;
                }
                file << "</text>\n";
            }

            // Draw drilling path
            if (showPath && !tour.empty()) {
                file << "<g>\n";
                for (size_t i = 0; i < tour.size() - 1; ++i) {
                    int idx1 = tour[i];
                    int idx2 = tour[i + 1];
                    if (idx1 >= 0 && idx1 < static_cast<int>(points.size()) &&
                        idx2 >= 0 && idx2 < static_cast<int>(points.size())) {
                        int intensity = static_cast<int>((255 * i) / (tour.size() - 1));
                        file << "<line x1=\"" << points[idx1].first << "\" y1=\"" << points[idx1].second
                            << "\" x2=\"" << points[idx2].first << "\" y2=\"" << points[idx2].second
                            << "\" stroke=\"rgb(" << intensity << ",0," << (255 - intensity)
                            << ")\" stroke-width=\"0.8\"/>\n";
                    }
                }
                file << "</g>\n";
            }

            // Draw holes with numbers
            file << "<g>\n";
            for (size_t i = 0; i < points.size(); ++i) {
                const auto& point = points[i];
                // Draw hole
                file << "<circle cx=\"" << point.first << "\" cy=\"" << point.second
                    << "\" r=\"2\" fill=\"blue\"/>\n";
                // Add hole number
                file << "<text x=\"" << point.first + 2.5 << "\" y=\"" << point.second
                    << "\" font-family=\"Arial\" font-size=\"5\" fill=\"black\">"
                    << i << "</text>\n";
            }
            file << "</g>\n";

            file << "</svg>\n";
            file.close();
        }
        catch (const std::exception&) {
            // Silent error handling
        }
    }

    static void saveKeySnapshots(const std::vector<std::pair<double, double>>& points,
        const std::vector<int>& tour,
        const std::string& baseFilename,
        int iteration,
        double cost,
        bool isCalibration = true) {

        if (isCalibration) return;  // Skip visualization during calibration

        bool shouldSave = false;

        // Save at milestone iterations
        if (iteration == 0 || iteration == 100 || iteration == 500 ||
            iteration == 1000 || iteration == 1500 || iteration == 2000) {
            shouldSave = true;
        }

        // Save when we find a significantly better solution
        if (cost < getBestRecordedCost() - 0.01) {
            getBestRecordedCost() = cost;
            shouldSave = true;
        }

        if (shouldSave) {
            std::string filename = baseFilename + "_iter" +
                std::to_string(iteration) + ".svg";
            generateSVG(points, tour, filename, true, iteration, cost);
        }
    }
};

#endif