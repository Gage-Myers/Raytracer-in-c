#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "ppmrw.h"
#include "json.h"

// Function prototypes
void      normal(double *v);
double    square(double v);
double    dot_prod(double *v1, double *v2);
double    distance(double *p1, double *p2);
double    intersect_sphere(double *rO, double *rD, double radius, double *position);
double    intersect_plane(double *rO, double *rD, double *normal, double *position);

double square(double v) {
  return v * v;
}

 double dot_prod(double *v1, double *v2) {
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

 double distance(double *p1, double *p2) {
  return sqrt(square(p2[0] - p1[0]) + square(p2[1] - p1[1]) + square(p2[2] - p1[2]));
}

 void normal(double *v) {
  double length = sqrt(square(v[0]) + square(v[1]) + square(v[2]));
  v[0] = v[0] / length;
  v[1] = v[1] / length;
  v[2] = v[2] / length;
}

 double intersect_plane(double *rO, double *rD, double *normal, double *position) {
  double origin[3] = {0, 0, 0};
  double d = distance(origin, position);
  double vD = dot_prod(normal, rD);
  if (vD == 0) {
    return -1;
  }
  double v0 = -1 * (dot_prod(normal, rO) + d);
  double intersect = v0 / vD;
  // Intersection is out of viewplane
  if (intersect < 0) {
    return -1;
  } else {
    return intersect;
  }
}

 double intersect_sphere(double *rO, double *rD,
                                     double radius, double *position) {

  // Solve for t using Quadratic Equation
  double A = square(rD[0]) + square(rD[1]) + square(rD[2]);
  double B = 2 * (rD[0] * (rO[0] - position[0]) +
                  rD[1] * (rO[1] - position[1]) +
                  rD[2] * (rO[2] - position[2]));
  double C = square(rO[0] - position[0]) + square(rO[1] - position[1]) +
             square(rO[2] - position[2]) - square(radius);
  double det = square(B) - 4 * A * C;

  // If discriminant is negative do NOT compute
  if (det < 0) {
    return -1;
  }
  // Find which t is closer to the ray
  double t0 = (-B - sqrt(det)) / 2;
  if (t0 > 0) {
    return t0;
  }
  double t1 = (-B + sqrt(det)) / 2;
  if (t1 > 0) {
    return t1;
  }
  // No intersect
  return -1;
}
