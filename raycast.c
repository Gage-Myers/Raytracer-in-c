#include "raycast.h"

int main(int argc, char const *argv[]) {

  // Ensures the correct number of args were passed in
  if (argc != 5) {
    fprintf(stderr, "\nERROR: Excpected 5 arguments. %d were given.", argc);
    exit(1);
  }

  // Checks width and height
  if ((atoi(argv[1]) == 0 && *argv[1] != '0') ||
      (*argv[2] != '0' && atoi(argv[2]) == 0)) {
    fprintf(stderr, "\nERROR: Input height or width are not numerical.\n");
    exit(1);
  }

  // Check that file exists
  FILE *inFile = fopen(argv[3], "r");
  if (!inFile) {
    fprintf(stderr, "\nERROR: Input file does not exist.\n");
    exit(1);
  }

  // Creates pixel buffer
  int M = atoi(argv[2]);
  int N = atoi(argv[1]);
  pixel *pixBuffer = (pixel *) malloc(sizeof(pixel) * N * M);
  fill(pixBuffer, &N, &M);


  // finds the size of the file
  fseek(inFile, 0L, SEEK_END);
  int bufferSize = ftell(inFile);
  char *buffer = (char *) malloc(sizeof(char) * bufferSize);
  entity *entities = (entity *) malloc(sizeof(entity) * 128 + 1);
  memset(buffer, 0, sizeof(buffer));
  rewind(inFile);

  // Read in the file to the buffer
  read_buffer(inFile, buffer, &bufferSize);
  fclose(inFile);

  // Read in the scene from the buffer
  int objCount = read_scene(buffer, &bufferSize, entities);
  free(buffer);

  // If no camera or entities were supplied, write a blank file
  if (!objCount) {
    FILE *outFile = fopen(argv[4], "w");
    fclose(outFile);
    printf("No camera or entities supplied. File will be written as blank.");
    return 1;
  }

  // raycast and write to scene
  double cx = 0;
  double cy = 0;
  double h = entities[0].camera.height;
  double w = entities[0].camera.width;
  double pixHeight = h / M;
  double pixWidth = w / N;
  int y, x;
  int i;
  int j = N * M;

  // This loop writes the scene to the file
  for (y = 0; y < M; y++) {
    for (x = 0; x < N; x++) {
      double rO[3] = {0, 0, 0};
      double rD[3] = {
        cx - (w/2) + pixWidth * (x + 0.5),
        cy - (h/2) + pixHeight * (y + 0.5),
        1
      };
      normal(rD);

      double closest = INFINITY;

      for (i = 1; i < objCount; i++) {
        double t = 0;
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
            exit(1);
        }
        if (t > 0 && t < closest) {
          closest = t;
          // Account for intersection and write color accordingly
          if (type == 2) {
            pixBuffer[j].r = 255 * (entities[i].sphere.color[0]);
            pixBuffer[j].g = 255 * (entities[i].sphere.color[1]);
            pixBuffer[j].b = 255 * (entities[i].sphere.color[2]);
          } else {
            pixBuffer[j].r = 255 * (entities[i].plane.color[0]);
            pixBuffer[j].g = 255 * (entities[i].plane.color[1]);
            pixBuffer[j].b = 255 * (entities[i].plane.color[2]);
          }
        }
      }
      --j;
    }
  }

  // write to file
  FILE *outFile = fopen(argv[4], "wb");
  int ver = 6;
  int colorMax = 255;
  write6(outFile, pixBuffer, &N, &M, &colorMax, &ver);
  fclose(outFile);

  return 0;
}
