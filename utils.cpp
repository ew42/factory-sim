#include "utils.h"

NormDist::NormDist(double m, double s) 
    : rng(1), mean(m), std(s), dist(m, s) {}

double NormDist::sample() {
    double x;
    do {
        x = dist(rng);
    }
    while(x < 0.0); // kills all negative rolls
    return x;
}

