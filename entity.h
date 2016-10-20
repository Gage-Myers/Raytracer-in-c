

#define ENTITY

typedef struct {
    int type;
    union {
        struct {
            double *position;
            double *normal;
            double *color;
        } plane; // Type: 3

        struct {
            double radius;
            double *position;
            double *color;
        } sphere; // Type: 2

        struct {
            double width;
            double height;
        } camera; // Type: 1
    };
} entity;
