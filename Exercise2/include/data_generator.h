#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include <vector>
#include <string>
#include <random>
#include <cmath>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

enum class BoardPattern {
    DIP_IC,        // Dual In-line Package / Integrated Circuit
    SOIC,          // Small Outline Integrated Circuit
    CONNECTOR,     // Edge connector
    MOUNTING,      // Mounting holes
    VIA,           // Through-hole vias
    CUSTOM         // Custom pattern
};

class TSPGenerator {
public:
    // Standard manufacturing constants
    static constexpr double MIN_HOLE_DIAMETER = 0.8;    // mm
    static constexpr double MIN_HOLE_SPACING = 2.54;    // mm (0.1 inch standard)
    static constexpr double MOUNTING_HOLE_SIZE = 3.2;   // mm
    static constexpr double EDGE_MARGIN = 5.0;          // mm

    struct Point {
        double x, y;
        Point(double _x = 0, double _y = 0) : x(_x), y(_y) {}

        double distance(const Point& other) const {
            double dx = x - other.x;
            double dy = y - other.y;
            return std::sqrt(dx * dx + dy * dy);
        }
    };

    struct Component {
        BoardPattern type;
        std::vector<Point> holes;
        double min_spacing;
        std::string description;
    };

    static std::vector<Component> createStandardPatterns(double board_width, double board_height) {
        return {
            // DIP-14 package (common for basic ICs)
            {BoardPattern::DIP_IC,
             {Point(0,0),  Point(0,2.54),  Point(0,5.08),  Point(0,7.62),  Point(0,10.16),  Point(0,12.7),  Point(0,15.24),
              Point(7.62,0), Point(7.62,2.54), Point(7.62,5.08), Point(7.62,7.62), Point(7.62,10.16), Point(7.62,12.7), Point(7.62,15.24)},
             2.54,
             "14-pin DIP IC"},

             // SOIC-8 package
             {BoardPattern::SOIC,
              {Point(0,0), Point(0,1.27), Point(0,2.54), Point(0,3.81),
               Point(5.08,0), Point(5.08,1.27), Point(5.08,2.54), Point(5.08,3.81)},
              1.27,
              "8-pin SOIC"},

              // Standard edge connector
              {BoardPattern::CONNECTOR,
               {Point(0,0), Point(2.54,0), Point(5.08,0), Point(7.62,0), Point(10.16,0)},
               2.54,
               "5-pin Edge Connector"},

               // Mounting holes
               {BoardPattern::MOUNTING,
                {Point(EDGE_MARGIN, EDGE_MARGIN),
                 Point(board_width - EDGE_MARGIN, EDGE_MARGIN),
                 Point(EDGE_MARGIN, board_height - EDGE_MARGIN),
                 Point(board_width - EDGE_MARGIN, board_height - EDGE_MARGIN)},
                10.0,
                "Mounting Holes"}
        };
    }

    // Circuit board specific generator
    static std::vector<std::vector<double>> generateCircuitBoard(
        double board_width = 100,
        double board_height = 100,
        int num_components = 5,
        unsigned seed = 0) {

        std::mt19937 rng(seed ? seed : std::random_device{}());
        std::vector<Point> hole_positions;
        std::string board_info;

        // Get standard component patterns
        auto patterns = createStandardPatterns(board_width, board_height);

        // Always place mounting holes first
        const auto& mounting = patterns[3];
        for (const auto& hole : mounting.holes) {
            hole_positions.push_back(hole);
        }
        board_info += "Added mounting holes at corners\n";

        // Place components with manufacturing constraints
        std::uniform_real_distribution<double> x_dist(EDGE_MARGIN, board_width - EDGE_MARGIN);
        std::uniform_real_distribution<double> y_dist(EDGE_MARGIN, board_height - EDGE_MARGIN);
        std::uniform_int_distribution<int> pattern_dist(0, 2); // DIP, SOIC, or connector

        int successful_placements = 0;
        for (int i = 0; i < num_components && successful_placements < 50; i++) {
            const auto& pattern = patterns[pattern_dist(rng)];
            bool valid = false;
            int attempts = 0;
            Point offset;

            while (!valid && attempts++ < 100) {
                valid = true;
                offset = Point(x_dist(rng), y_dist(rng));

                // Check manufacturing constraints
                for (const auto& pattern_hole : pattern.holes) {
                    Point new_hole(offset.x + pattern_hole.x,
                        offset.y + pattern_hole.y);

                    // Check board boundaries
                    if (new_hole.x < EDGE_MARGIN || new_hole.x > board_width - EDGE_MARGIN ||
                        new_hole.y < EDGE_MARGIN || new_hole.y > board_height - EDGE_MARGIN) {
                        valid = false;
                        break;
                    }

                    // Check spacing with existing holes
                    for (const auto& existing : hole_positions) {
                        if (new_hole.distance(existing) < pattern.min_spacing) {
                            valid = false;
                            break;
                        }
                    }
                    if (!valid) break;
                }
            }

            if (valid) {
                for (const auto& pattern_hole : pattern.holes) {
                    hole_positions.push_back(Point(
                        offset.x + pattern_hole.x,
                        offset.y + pattern_hole.y
                    ));
                }
                successful_placements++;
                board_info += "Added " + pattern.description + " at (" +
                    std::to_string(offset.x) + "," + std::to_string(offset.y) + ")\n";
            }
        }

        // Generate distance matrix with manufacturing precision
        int N = hole_positions.size();
        std::vector<std::vector<double>> costs(N, std::vector<double>(N));

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (i != j) {
                    costs[i][j] = hole_positions[i].distance(hole_positions[j]);
                }
            }
        }

        // Save board info for debugging/visualization
        saveToFile("board_layout.txt", costs,
            "Board dimensions: " + std::to_string(board_width) + "x" +
            std::to_string(board_height) + " mm\n" + board_info);

        return costs;
    }



    // Generate random instances
    static std::vector<std::vector<double>> generateRandom(int N, double min_cost = 1.0,
        double max_cost = 100.0,
        unsigned seed = 0) {
        std::mt19937 rng(seed ? seed : std::random_device{}());
        std::uniform_real_distribution<double> dist(min_cost, max_cost);

        std::vector<std::vector<double>> costs(N, std::vector<double>(N));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (i != j) {
                    costs[i][j] = dist(rng);
                    costs[j][i] = costs[i][j]; // Ensure symmetry
                }
            }
        }
        return costs;
    }

    // Save instance to file
    static void saveToFile(const std::string& filename,
        const std::vector<std::vector<double>>& costs,
        const std::string& metadata = "") {

        std::ofstream out(filename);
        if (!out) throw std::runtime_error("Cannot open file: " + filename);

        int N = costs.size();
        out << N << "\n";

        // Optional metadata
        if (!metadata.empty()) {
            out << "# " << metadata << "\n";
        }

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                out << std::fixed << std::setprecision(6) << costs[i][j] << " ";
            }
            out << "\n";
        }
    }

    // Load instance from file
    static std::vector<std::vector<double>> loadFromFile(const std::string& filename) {
        std::ifstream in(filename);
        if (!in) throw std::runtime_error("Cannot open file: " + filename);

        int N;
        in >> N;

        std::vector<std::vector<double>> costs(N, std::vector<double>(N));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                in >> costs[i][j];
            }
        }
        return costs;
    }
    private:
        static bool isValidPosition(const Point& p, double board_width, double board_height) {
            return p.x >= EDGE_MARGIN && p.x <= board_width - EDGE_MARGIN &&
                p.y >= EDGE_MARGIN && p.y <= board_height - EDGE_MARGIN;
        }
};

#endif