#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ppmrw.h"
#include "json.h"

/* Function prototypes */
void      normal(double *v);
double    square(double v);
void      add_vect(double *v1, double *v2, double *v3);
void      sub_vect(double *v1, double *v2, double *v3);
void      scale_double(double factor, double *v1, double *v2);
double    dot_prod(double *v1, double *v2);
double    distance(double *p1, double *p2);
void      reflect(double *L, double *N, double *R);
double    intersect_sphere(double *rO, double *rD,
                                      double radius, double *position);
double    intersect_plane(double *rO, double *rD,
                                     double *normal, double *position);
double    f_radial(entity obj, double distance);
double    f_angular(entity obj, double *vector);
void      diffuse_reflection(double *N, double *L,
                                        double *lColor, double *oColor,
                                        double *diff);
void      specular_reflection(double factor, double *L,
                                         double *R, double *N, double *V,
                                         double *lColor, double *oColor,
                                         double *spec);
void      copy_vector(double *v1, double *v2);


// Diffuese reflection on a surface

void diffuse_reflection(double *N, double *L, double *lColor,
                                     double *oColor, double *diff) {
  double temp = dot_prod(N, L);
  if (temp > 0) {
    double *temp2 = malloc(sizeof(double) * 3);
    temp2[0] = lColor[0] * oColor[0] * temp;
    temp2[1] = lColor[1] * oColor[1] * temp;
    temp2[2] = lColor[2] * oColor[2] * temp;
    copy_vector(temp2, diff);
  } else {
    diff[0] = 0.0;
    diff[1] = 0.0;
    diff[2] = 0.0;
  }
}

 // Copy a vector into another variable

 void copy_vector(double *v1, double *v2) {
  v2[0] = v1[0];
  v2[1] = v1[1];
  v2[2] = v1[2];
}

// Specular reflection off of an entity
 void specular_reflection(double factor, double *L, double *R,
                                      double *N, double *V, double *lColor,
                                      double *oColor, double *spec) {
  double VR = dot_prod(V, R);
  double NL = dot_prod(N, L);
  if (VR > 0 && NL > 0) {
    double temp = pow(VR, factor);
    double *temp2 = malloc(sizeof(double) * 3);
    temp2[0] = lColor[0] * oColor[0] * temp;
    temp2[1] = lColor[1] * oColor[1] * temp;
    temp2[2] = lColor[2] * oColor[2] * temp;
    copy_vector(temp2, spec);
  } else {
    spec[0] = 0.0;
    spec[1] = 0.0;
    spec[2] = 0.0;
  }
}

// Gives the angular attribute of a light

 double f_angular(entity obj, double *vector) {
  if (obj.type == 4) {
    return 1.0;
  } else {
    double temp = dot_prod(obj.light.spotlight.direction, vector);
    if (temp > cos((obj.light.spotlight.theta * M_PI) / 180 )) {
      temp = 0.0;
     } else {
      temp = pow(temp, obj.light.spotlight.ang_a0);
    }
    return temp;
  }
}

// Gives the radial attribute of a light

 double f_radial(entity obj, double distance) {
  return (1.0 / (obj.light.rad_a2 * square(distance) +
                obj.light.rad_a1 * distance +
                obj.light.rad_a0));
}

//  Adds 2 vectors

 void add_vect(double *v1, double *v2, double *v3) {
  v3[0] = v1[0] + v2[0];
  v3[1] = v1[1] + v2[1];
  v3[2] = v1[2] + v2[2];
}

// Reflects a vector

 void reflect(double *L, double *N, double *R) {
  double temp = 2 * dot_prod(L, N);
  double *temp2 = malloc(sizeof(double) * 3);
  scale_double(temp, N, temp2);
  sub_vect(temp2, L, R);
}

// Subtracts 2 vectors

 void sub_vect(double *v1, double *v2, double *v3) {
  v3[0] = v1[0] - v2[0];
  v3[1] = v1[1] - v2[1];
  v3[2] = v1[2] - v2[2];
}

// Scales a vector by a passed in factor

 void scale_double(double factor, double *v1, double *v2) {
  v2[0] = v1[0] * factor;
  v2[1] = v1[1] * factor;
  v2[2] = v1[2] * factor;
}

// Finds the square of a double

 double square(double v) {
  return v * v;
}

// Returns the dot product of two vectors

 double dot_prod(double *v1, double *v2) {
  return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

// Determines the distance between two points

 double distance(double *p1, double *p2) {
  return sqrt(square(p2[0] - p1[0]) + square(p2[1] - p1[1]) +
              square(p2[2] - p1[2]));
}

// Finds a vectors normal

 void normal(double *v) {
  double length = sqrt(square(v[0]) + square(v[1]) + square(v[2]));
  double *temp = malloc(sizeof(double) * 3);
  temp[0] = v[0] / length;
  temp[1] = v[1] / length;
  temp[2] = v[2] / length;
  copy_vector(temp, v);
}

// Determines if a ray intersects with a given plane

 double intersect_plane(double *rO, double *rD,
                                    double *normal, double *position) {
  double origin[3] = {0, 0, 0};
  double d = distance(origin, position);
  double vD = dot_prod(normal, rD);
  /* If 0 the plane is parallel - so no intersection */
  if (vD == 0) {
    return -1;
  }
  double v0 = -1 * (dot_prod(normal, rO) + d);
  double intersect = v0 / vD;
  /* Intersection is out of viewplane is < 0 */
  if (intersect < 0) {
    return -1;
  } else {
    return intersect;
  }
}

// Determines if a ray intersects with a sphere

 double intersect_sphere(double *rO, double *rD,
                                     double radius, double *position) {
  /* Solve for t using Quadratic Equation */
  double A = square(rD[0]) + square(rD[1]) + square(rD[2]);
  double B = 2 * (rD[0] * (rO[0] - position[0]) +
                  rD[1] * (rO[1] - position[1]) +
                  rD[2] * (rO[2] - position[2]));
  double C = square(rO[0] - position[0]) + square(rO[1] - position[1]) +
             square(rO[2] - position[2]) - square(radius);
  double det = square(B) - 4 * A * C;

  /* If discriminant is negative do NOT compute */
  if (det < 0) {
    return -1;
  }
  /* Find which t is closer to the ray */
  double t0 = (-B - sqrt(det)) / 2;
  if (t0 > 0) {
    return t0;
  }
  double t1 = (-B + sqrt(det)) / 2;
  if (t1 > 0) {
    return t1;
  }
  /* Otherwise no intersection */
  return -1;
}
