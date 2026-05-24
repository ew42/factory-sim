#ifndef UTILS_H
#define UTILS_H

#include <random>

struct NormDist {
    /* 
     * Average
     * Standard Deviation
     */
    std::mt19937_64 rng;
    double mean_;
    double std_;
    std::normal_distribution<double> dist;

    NormDist(double m, double s);
    double sample();
	double mean() const;
	double std() const;
};

#endif // UTILS_H

