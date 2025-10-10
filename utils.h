#ifndef UTILS_H
#define UTILS_H

#include <random>

struct NormDist {
    /* 
     * Average
     * Standard Deviation
     */
    std::mt19937_64 rng;
    double mean;
    double std;
    std::normal_distribution<double> dist;

    NormDist(double m, double s);
    double sample();
};

#endif // UTILS_H

