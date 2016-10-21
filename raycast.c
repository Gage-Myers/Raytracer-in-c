#include "raycast.h"

int main(int argc, char const *argv[]) {

  /* Verify the correct number of arguments supplied */
  if (argc != 5) {
    fprintf(stderr, "\nERROR: Incorrect number of arguments."
                    " Expected five arguments, %d supplied."
                    "\nFormat: Program_Name Width Height File_In File_Out.\n"
                    , argc);
    exit(1);
  }

  /* Verify valid image width and height supplied */
  if ((atoi(argv[1]) == 0 && *argv[1] != '0') ||
      (*argv[2] != '0' && atoi(argv[2]) == 0)) {
    fprintf(stderr, "\nERROR: Input height or width are not numerical.\n");
    exit(1);
  }

  /* Verify that the input file exists */
  FILE *in_file = fopen(argv[3], "r");
  if (!in_file) {
    fprintf(stderr, "\nERROR: Input file does not exist.\n");
    exit(1);
  }

  /* Get size of JSON file to initiate buffer */
  fseek(in_file, 0L, SEEK_END);
  int bufferSize  = ftell(in_file);
  char *buffer    = (char *) malloc(sizeof(char) * bufferSize);
  entity *entities = (entity *) malloc(sizeof(entity) * 128 + 1);
  entity *lights  = (entity *) malloc(sizeof(entity) * 128 + 1);
  memset(buffer, 0, sizeof(buffer));
  rewind(in_file);

  /* Read in the file to the buffer */
  int bufLen = read_buffer(in_file, buffer, &bufferSize);
  fclose(in_file);

  /* Read in the scene from the buffer */
  int *ent_count = malloc(sizeof(int) * 2);
  read(buffer, &bufLen, entities, lights, ent_count);
  free(buffer);

  /* If no camera or entities were supplied, write a blank file */
  if (!ent_count) {
    FILE *out_file = fopen(argv[4], "w");
    fclose(out_file);
    printf("No camera or entities supplied. File will be written as blank.");
  }

  /* Initialize the pixel buffer, and image dimensions, and color it black */
  int M = atoi(argv[2]);
  int N = atoi(argv[1]);
  pixel *pixBuffer = (pixel *) malloc(sizeof(pixel) * N * M);

  /* Raycast and write scene to pixel buffer */
  double cx = 0;
  double cy = 0;
  double h = entities[0].camera.height;
  double w = entities[0].camera.width;
  double pixHeight = h / M;
  double pixWidth = w / N;
  int y, x;
  int i, j, k;

  /* Write the scene to file*/
  for (y = 0; y < M; y++) {
    /* For each pixel in the buffer */
    for (x = 0; x < N; x++) {
      double rO[3] = {0, 0, 0};
      double rD[3] = {
        cx - (w/2) + pixWidth * (x + 0.5),
        cy - (h/2) + pixHeight * (y + 0.5),
        1
      };
      normal(rD);

      double closest = INFINITY;
      double t       = 0;
      int closestObj;
      /* For each entity see if intersection at pixel */
      for (i = 1; i < ent_count[0]; i++) {
        int type = entities[i].type;
        switch(type) {
          case 2:
            t = intersect_sphere(rO, rD,
                                entities[i].sphere.radius,
                                entities[i].sphere.position);
            break;
          case 3:
            t = intersect_plane(rO, rD,
                               entities[i].plane.normal,
                               entities[i].plane.position);
            break;
          default:
            fprintf(stderr, "\nERROR: Unexpected error.\n");
            exit(1);
        }
        /* Select the closest entity from the ray that was casted */
        if (t > 0 && t < closest) {
          closest    = t;
          closestObj = i;
        }
      }

      /* Ambient light's color */
      double *color = malloc(sizeof(double) * 3);
      color[0] = 0.0;
      color[1] = 0.0;
      color[2] = 0.0;
      double *rOn = malloc(sizeof(double) * 3);
      double *rDn = malloc(sizeof(double) * 3);
      double lightDistance;
      int shadowed = 0;

      for (j = 0; j < ent_count[1]; j++) {
          /* Calculate rOn */
          double *temp = malloc(sizeof(double) * 3);
          scale_double(closest, rD, temp);
          add_vect(temp, rO, rOn);

          /* Calculate rDn */
          sub_vect(lights[j].light.position, rOn, rDn);
          normal(rDn);

          /* Calculate light distance */
          lightDistance = distance(rOn, lights[j].light.position);

          for (k = 1; k < ent_count[0]; k++) {
            if (k == closestObj) {
              continue;
            }
            switch (entities[k].type) {
              case 2:
                t = intersect_sphere(rOn, rDn,
                                    entities[k].sphere.radius,
                                    entities[k].sphere.position);
                break;
              case 3:
                t = intersect_plane(rOn, rDn,
                                   entities[k].plane.normal,
                                   entities[k].plane.position);
                break;
              default:
                fprintf(stderr, "\nERROR: Unexpected error.\n");
                exit(1);
            }
            if (t >= 0 && t < lightDistance) {
              shadowed = 1;
            }
          }

          if (!shadowed && closest >= 0) {
            double *pN = malloc(sizeof(double) * 3);
            double *L  = malloc(sizeof(double) * 3);
            copy_vector(rDn, L);
            normal(L);
            double *V  = malloc(sizeof(double) * 3);
            copy_vector(rD, V);
            normal(V);
            double *R  = malloc(sizeof(double) * 3);
            double *diffuse = malloc(sizeof(double) * 3);
            double *specular = malloc(sizeof(double) * 3);
            switch (entities[closestObj].type) {
              case 2:
                sub_vect(rOn, entities[closestObj].sphere.position, pN);
                copy_vector(entities[closestObj].sphere.diffuse_color, diffuse);
                copy_vector(entities[closestObj].sphere.specular_color, specular);
                break;
              case 3:
                copy_vector(entities[closestObj].plane.normal, pN);
                copy_vector(entities[closestObj].plane.diffuse_color, diffuse);
                copy_vector(entities[closestObj].plane.specular_color, specular);
                break;
              default:
                fprintf(stderr, "\nERROR: Unexpected error.\n");
                exit(1);
                break;
            }
            normal(pN);
            reflect(L, pN, R);
            normal(R);
            scale_double(-1, V, V);
            specular_reflection(20, L, R, pN, V, lights[j].light.color,
                               specular, specular);
            diffuse_reflection(pN, L, lights[j].light.color,
                               diffuse, diffuse);
            double *sRdn = malloc(sizeof(double) * 3);
            scale_double(-1, rDn, sRdn);
            color[0] += (diffuse[0] + specular[0]) *
                        f_radial(lights[j], lightDistance) *
                        f_angular(lights[j], sRdn);
            color[1] += (diffuse[1] + specular[1]) *
                        f_radial(lights[j], lightDistance) *
                        f_angular(lights[j], sRdn);
            color[2] += (diffuse[2] + specular[2]) *
                        f_radial(lights[j], lightDistance) *
                        f_angular(lights[j], sRdn);
          }
      }

      pixBuffer[((M-y-1) * N + x)].r = (unsigned char) (255 * clamp(color[0]));
      pixBuffer[((M-y-1) * N + x)].g = (unsigned char) (255 * clamp(color[1]));
      pixBuffer[((M-y-1) * N + x)].b = (unsigned char) (255 * clamp(color[2]));

    }
  }

  /* Write pixel buffer to file */
  FILE *out_file = fopen(argv[4], "wb");
  int ver = 6;
  int colorMax = 255;
  write6(out_file, pixBuffer, &N, &M, &colorMax, &ver);
  fclose(out_file);

  return 0;
}
