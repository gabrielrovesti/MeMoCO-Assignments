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

    static void writeSearchMetrics(std::ofstream& file, double x, double y,
        int iteration, double currentCost, int tenure = -1) {
        file << "<text x=\"" << x << "\" y=\"" << y
            << "\" font-family=\"Arial\" font-size=\"10\" fill=\"black\">";
        file << "Iteration: " << iteration;
        if (currentCost >= 0) {
            file << " | Cost: " << std::fixed << std::setprecision(2) << currentCost;
        }
        if (tenure >= 0) {
            file << " | Tenure: " << tenure;
        }
        file << "</text>\n";
    }

    static void drawPath(std::ofstream& file,
        const std::vector<std::pair<double, double>>& points,
        const std::vector<int>& tour,
        bool useGradient = true) {

        file << "<g>\n";
        for (size_t i = 0; i < tour.size() - 1; ++i) {
            int idx1 = tour[i];
            int idx2 = tour[i + 1];
            if (idx1 >= 0 && idx1 < static_cast<int>(points.size()) &&
                idx2 >= 0 && idx2 < static_cast<int>(points.size())) {

                int intensity = useGradient ?
                    static_cast<int>((255 * i) / (tour.size() - 1)) : 128;

                file << "<line "
                    << "x1=\"" << points[idx1].first << "\" "
                    << "y1=\"" << points[idx1].second << "\" "
                    << "x2=\"" << points[idx2].first << "\" "
                    << "y2=\"" << points[idx2].second << "\" "
                    << "stroke=\"rgb(" << intensity << ",0," << (255 - intensity)
                    << ")\" stroke-width=\"0.8\"/>\n";
            }
        }
        file << "</g>\n";
    }

public:
    static void generateSVG(const std::vector<std::pair<double, double>>& points,
        const std::vector<int>& tour,
        const std::string& filename,
        bool showPath = true,
        int iteration = -1,
        double currentCost = -1.0,
        int tenure = -1) {

        try {
            std::filesystem::path filePath(filename);
            std::filesystem::path dir = filePath.parent_path();

            if (!dir.empty() && !std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);
            }

            std::ofstream file(filename);
            if (!file.is_open()) return;

            // Calculate bounds (keeping the exact same calculation method)
            double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
            for (size_t i = 0; i < points.size(); ++i) {
                if (points[i].first < minX) minX = points[i].first;
                if (points[i].second < minY) minY = points[i].second;
                if (points[i].first > maxX) maxX = points[i].first;
                if (points[i].second > maxY) maxY = points[i].second;
            }

            double margin = 10.0;
            minX -= margin;
            minY -= margin;
            maxX += margin;
            maxY += margin;

            // SVG header and background
            file << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                << "viewBox=\"" << minX << " " << minY << " "
                << (maxX - minX) << " " << (maxY - minY) << "\">\n"
                << "<rect x=\"" << minX << "\" y=\"" << minY
                << "\" width=\"" << (maxX - minX)
                << "\" height=\"" << (maxY - minY)
                << "\" fill=\"white\"/>\n";

            // Add metrics if available
            if (iteration >= 0) {
                writeSearchMetrics(file, minX + 5, minY + 15,
                    iteration, currentCost, tenure);
            }

            // Draw drilling path
            if (showPath && !tour.empty()) {
                drawPath(file, points, tour);
            }

            // Draw holes with labels
            file << "<g>\n";
            for (size_t i = 0; i < points.size(); ++i) {
                const auto& point = points[i];
                // Draw hole
                file << "<circle cx=\"" << point.first
                    << "\" cy=\"" << point.second
                    << "\" r=\"2\" fill=\"blue\"/>\n"
                    << "<text x=\"" << point.first + 2.5
                    << "\" y=\"" << point.second
                    << "\" font-family=\"Arial\" font-size=\"5\" "
                    << "fill=\"black\">" << i << "</text>\n";
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

    // New method for side-by-side comparison visualization
    static void generateComparisonSVG(
        const std::vector<std::pair<double, double>>& points,
        const std::vector<int>& initialTour,
        const std::vector<int>& finalTour,
        const std::string& filename,
        double initialCost,
        double finalCost) {

        try {
            std::ofstream file(filename);
            if (!file.is_open()) return;

            // Calculate bounds (using the same method)
            double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
            for (size_t i = 0; i < points.size(); ++i) {
                if (points[i].first < minX) minX = points[i].first;
                if (points[i].second < minY) minY = points[i].second;
                if (points[i].first > maxX) maxX = points[i].first;
                if (points[i].second > maxY) maxY = points[i].second;
            }

            double margin = 10.0;
            minX -= margin;
            minY -= margin;
            maxX += margin;
            maxY += margin;

            // Double the width for side-by-side display
            double width = (maxX - minX) * 2.2;
            double height = maxY - minY;

            file << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                << "viewBox=\"" << minX << " " << minY << " "
                << width << " " << height << "\">\n";

            // Left side: Initial solution
            file << "<g transform=\"translate(0,0)\">\n";
            file << "<rect x=\"" << minX << "\" y=\"" << minY
                << "\" width=\"" << (maxX - minX)
                << "\" height=\"" << height
                << "\" fill=\"white\"/>\n";
            drawPath(file, points, initialTour, false);
            file << "<text x=\"" << minX + 5 << "\" y=\"" << minY + 15
                << "\" font-family=\"Arial\" font-size=\"10\">"
                << "Initial Solution (Cost: " << std::fixed
                << std::setprecision(2) << initialCost << ")"
                << "</text>\n";
            file << "</g>\n";

            // Right side: Final solution
            file << "<g transform=\"translate(" << (maxX - minX) * 1.1 << ",0)\">\n";
            file << "<rect x=\"" << minX << "\" y=\"" << minY
                << "\" width=\"" << (maxX - minX)
                << "\" height=\"" << height
                << "\" fill=\"white\"/>\n";
            drawPath(file, points, finalTour, false);
            file << "<text x=\"" << minX + 5 << "\" y=\"" << minY + 15
                << "\" font-family=\"Arial\" font-size=\"10\">"
                << "Final Solution (Cost: " << std::fixed
                << std::setprecision(2) << finalCost << ")"
                << "</text>\n";
            file << "</g>\n";

            file << "</svg>\n";
            file.close();
        }
        catch (const std::exception&) {
            // Silent error handling
        }
    }
};

#endif