// A structure that will hold enitities
// read in from a JSON file

typedef struct {
    int type;
    union {
        struct {
            double *position;
            double *normal;
            double *diffuse_color;
            double *specular_color;
        } plane; // Type: 3

        struct {
            double radius;
            double *position;
            double *diffuse_color;
            double *specular_color;
        } sphere; // Type: 2

        struct {
            double width;
            double height;
        } camera; // Type: 1

        // This final struct is a dynamic
        // Light that will be able to be
        // to either a spot or radial light

        struct {
            double *color;
            double *position;
            double rad_a0;
            double rad_a1;
            double rad_a2;
            union {
              double *direction;
              double ang_a0;
              double theta;
            } spotlight;
          } light;
      };
} entity;
