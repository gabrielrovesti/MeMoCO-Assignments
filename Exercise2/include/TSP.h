#ifndef TSP_H
#define TSP_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

class TSP {
public:
    TSP() : n(0), infinite(1e10) {}
    int n;  // number of nodes/holes
    std::vector<std::vector<double>> cost;  // cost matrix
    double infinite;  // upper bound value for invalid solutions

    void read(const char* filename);
};

#endif /* TSP_H */