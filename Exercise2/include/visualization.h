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

using namespace std;

namespace {
    double best_recorded_cost = numeric_limits<double>::infinity();
}

class BoardVisualizer {
private:
    // Visualization constants
    static constexpr double BASE_SVG_SIZE = 800.0;
    static constexpr double BASE_MARGIN = 50.0;
    static constexpr double POINT_RADIUS = 8.0;
    static constexpr double TEXT_SIZE = 12.0;
    static constexpr double TEXT_OFFSET_X = 15.0;
    static constexpr double TEXT_OFFSET_Y = 5.0;
    static constexpr double PATH_STROKE_WIDTH = 2.0;

    static double& getBestRecordedCost() {
        return best_recorded_cost;
    }

    static void calculateBounds(const vector<pair<double, double>>& points,
        double& minX, double& minY, double& maxX, double& maxY) {
        if (points.empty()) return;

        // Initialize with first point
        minX = points[0].first;
        minY = points[0].second;
        maxX = points[0].first;
        maxY = points[0].second;

        for (size_t i = 1; i < points.size(); ++i) {
            if (points[i].first < minX) minX = points[i].first;
            if (points[i].second < minY) minY = points[i].second;
            if (points[i].first > maxX) maxX = points[i].first;
            if (points[i].second > maxY) maxY = points[i].second;
        }
    }

    static void calculateScaling(double width, double height, double& scale) {
        double maxDim = width > height ? width : height;
        scale = maxDim > 0 ? BASE_SVG_SIZE / maxDim : 1.0;
    }

    static void writeSearchMetrics(ofstream& file, double x, double y,
        int iteration, double currentCost, double textSize) {
        file << "<text x=\"" << x << "\" y=\"" << y
            << "\" font-family=\"Arial\" font-size=\"" << textSize
            << "\" fill=\"black\">";
        if (iteration >= 0) {
            file << "Iteration: " << iteration;
        }
        if (currentCost >= 0) {
            file << " Cost: " << fixed << setprecision(2) << currentCost;
        }
        file << "</text>\n";
    }

    static void drawNodes(ofstream& file,
        const vector<pair<double, double>>& points,
        double scale) {
        file << "<g>\n";
        for (size_t i = 0; i < points.size(); ++i) {
            double x = points[i].first * scale;
            double y = points[i].second * scale;

            // Draw point
            file << "<circle cx=\"" << x << "\" cy=\"" << y
                << "\" r=\"" << POINT_RADIUS << "\" fill=\"blue\"/>\n";

            // Position text
            double textX = x + TEXT_OFFSET_X;
            double textY = y + TEXT_OFFSET_Y;

            file << "<text x=\"" << textX << "\" y=\"" << textY
                << "\" font-family=\"Arial\" font-size=\"" << TEXT_SIZE
                << "\" fill=\"black\">" << i << "</text>\n";
        }
        file << "</g>\n";
    }

    static void drawPath(ofstream& file,
        const vector<pair<double, double>>& points,
        const vector<int>& tour,
        double scale,
        bool useGradient = true) {

        if (tour.size() < 2) return;

        file << "<g>\n";
        for (size_t i = 0; i < tour.size() - 1; ++i) {
            int idx1 = tour[i];
            int idx2 = tour[i + 1];

            if (idx1 >= 0 && idx1 < static_cast<int>(points.size()) &&
                idx2 >= 0 && idx2 < static_cast<int>(points.size())) {

                int colorVal = useGradient ?
                    static_cast<int>((255.0 * i) / (tour.size() - 1)) : 128;

                double x1 = points[idx1].first * scale;
                double y1 = points[idx1].second * scale;
                double x2 = points[idx2].first * scale;
                double y2 = points[idx2].second * scale;

                file << "<line "
                    << "x1=\"" << x1 << "\" y1=\"" << y1
                    << "\" x2=\"" << x2 << "\" y2=\"" << y2
                    << "\" stroke=\"rgb(" << colorVal << ",0,"
                    << (255 - colorVal) << ")\" "
                    << "stroke-width=\"" << PATH_STROKE_WIDTH << "\"/>\n";
            }
        }
        file << "</g>\n";
    }

public:
    static void generateSVG(const vector<pair<double, double>>& points,
        const vector<int>& tour,
        const string& filename,
        bool showPath = true,
        int iteration = -1,
        double currentCost = -1.0) {

        try {
            filesystem::path filePath(filename);
            filesystem::path dir = filePath.parent_path();

            if (!dir.empty() && !filesystem::exists(dir)) {
                filesystem::create_directories(dir);
            }

            ofstream file(filename);
            if (!file.is_open()) return;

            // Calculate bounds
            double minX, minY, maxX, maxY;
            calculateBounds(points, minX, minY, maxX, maxY);

            // Add margins
            minX -= BASE_MARGIN;
            minY -= BASE_MARGIN;
            maxX += BASE_MARGIN;
            maxY += BASE_MARGIN;

            // Calculate scale
            double scale;
            calculateScaling(maxX - minX, maxY - minY, scale);

            // Calculate final dimensions
            double width = (maxX - minX) * scale;
            double height = (maxY - minY) * scale;

            // Write SVG header
            file << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                << "width=\"" << width << "\" height=\"" << height << "\" "
                << "viewBox=\"" << minX * scale << " " << minY * scale << " "
                << width << " " << height << "\">\n";

            // Background
            file << "<rect x=\"" << minX * scale << "\" y=\"" << minY * scale
                << "\" width=\"" << width << "\" height=\"" << height
                << "\" fill=\"white\"/>\n";

            // Draw path if requested
            if (showPath) {
                drawPath(file, points, tour, scale);
            }

            // Draw nodes
            drawNodes(file, points, scale);

            // Add metrics if available
            if (iteration >= 0 || currentCost >= 0) {
                writeSearchMetrics(file,
                    minX * scale + BASE_MARGIN / 2,
                    minY * scale + BASE_MARGIN / 2,
                    iteration, currentCost, TEXT_SIZE * 1.2);
            }

            file << "</svg>\n";
            file.close();
        }
        catch (const exception&) {
            // Silent error handling
        }
    }

    static void generateComparisonSVG(
        const vector<pair<double, double>>& points,
        const vector<int>& initialTour,
        const vector<int>& finalTour,
        const string& filename,
        double initialCost,
        double finalCost) {

        try {
            filesystem::path filePath(filename);
            filesystem::path dir = filePath.parent_path();

            if (!dir.empty() && !filesystem::exists(dir)) {
                filesystem::create_directories(dir);
            }

            ofstream file(filename);
            if (!file.is_open()) return;

            // Calculate bounds
            double minX, minY, maxX, maxY;
            calculateBounds(points, minX, minY, maxX, maxY);

            // Add margins
            minX -= BASE_MARGIN;
            minY -= BASE_MARGIN;
            maxX += BASE_MARGIN;
            maxY += BASE_MARGIN;

            // Double width for side-by-side display
            double singleWidth = maxX - minX;
            double height = maxY - minY;
            double width = singleWidth * 2.2; // 2.2 for spacing between views

            // Calculate scale
            double scale;
            calculateScaling(singleWidth, height, scale);

            file << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                << "width=\"" << width * scale << "\" height=\"" << height * scale << "\" "
                << "viewBox=\"" << minX * scale << " " << minY * scale << " "
                << width * scale << " " << height * scale << "\">\n";

            // Left side: Initial solution
            file << "<g transform=\"translate(0,0)\">\n"
                << "<rect x=\"" << minX * scale << "\" y=\"" << minY * scale
                << "\" width=\"" << singleWidth * scale
                << "\" height=\"" << height * scale
                << "\" fill=\"white\"/>\n";
            drawPath(file, points, initialTour, scale, false);
            drawNodes(file, points, scale);
            writeSearchMetrics(file,
                minX * scale + BASE_MARGIN / 2,
                minY * scale + BASE_MARGIN / 2,
                -1, initialCost, TEXT_SIZE * 1.2);
            file << "</g>\n";

            // Right side: Final solution
            file << "<g transform=\"translate(" << singleWidth * 1.1 * scale << ",0)\">\n"
                << "<rect x=\"" << minX * scale << "\" y=\"" << minY * scale
                << "\" width=\"" << singleWidth * scale
                << "\" height=\"" << height * scale
                << "\" fill=\"white\"/>\n";
            drawPath(file, points, finalTour, scale, false);
            drawNodes(file, points, scale);
            writeSearchMetrics(file,
                minX * scale + BASE_MARGIN / 2,
                minY * scale + BASE_MARGIN / 2,
                -1, finalCost, TEXT_SIZE * 1.2);
            file << "</g>\n";

            file << "</svg>\n";
            file.close();
        }
        catch (const exception&) {
            // Silent error handling
        }
    }

    static void saveKeySnapshots(
        const vector<pair<double, double>>& points,
        const vector<int>& tour,
        const string& baseFilename,
        int iteration,
        double cost,
        bool isCalibration = true) {

        if (isCalibration) return;

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
            string filename = baseFilename + "_iter" +
                to_string(iteration) + ".svg";
            generateSVG(points, tour, filename, true, iteration, cost);
        }
    }
};

#endif