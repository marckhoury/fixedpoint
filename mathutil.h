#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <cmath>

#define EPSILON 1E-6

double dist(double* v, double* u);
double dot(double* v, double* u);
double norm(double* v);
void normalize(double* v);
void cross_product(double* v, double* u, double* res);
void inverse(float* m, float* res);

#endif
