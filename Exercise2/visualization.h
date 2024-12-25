#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <fstream>
#include <sstream>
#include <vector>
#include <utility> 
#include "TSPSolution.h"
#include "TSP.h"
#include "data_generator.h"

class BoardVisualizer {
public:
    static void generateSVG(const std::vector<TSPGenerator::Point>& points,
        const std::vector<int>& tour,
        const std::string& filename,
        bool showPath = true) {
        // Calculate bounds
        double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
        for (const auto& point : points) {
            minX = std::min(minX, point.x);
            minY = std::min(minY, point.y);
            maxX = std::max(maxX, point.x);
            maxY = std::max(maxY, point.y);
        }

        // Add margins
        double margin = 5.0;
        minX -= margin;
        minY -= margin;
        maxX += margin;
        maxY += margin;

        std::ofstream file(filename);
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
        file << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            << "viewBox=\"" << minX << " " << minY << " "
            << (maxX - minX) << " " << (maxY - minY) << "\">\n";

        // Draw drill paths if requested
        if (showPath) {
            file << "<g stroke=\"red\" stroke-width=\"0.5\" fill=\"none\">\n";
            file << "<path d=\"M";
            for (size_t i = 0; i < tour.size(); ++i) {
                int idx = tour[i];
                file << points[idx].x << "," << points[idx].y;
                if (i < tour.size() - 1) file << " L ";
            }
            file << "\"/>\n</g>\n";
        }

        // Draw drill points
        file << "<g>\n";
        for (const auto& point : points) {
            file << "<circle cx=\"" << point.x << "\" cy=\"" << point.y
                << "\" r=\"1\" fill=\"blue\"/>\n";
        }
        file << "</g>\n";

        file << "</svg>";
        file.close();
    }
};

#endif