// Include entity file and standard libraries
#include "entity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
static inline  int     read_buffer(FILE *in, char *buffer, int *length);
static inline  int     get_entity_type(char *string);
static inline  char    *get_string(char *buffer, int *index, int *length);
static inline  double  *get_vect(char *buffer, int *index, int *length);
static inline  double  get_num(char *buffer, int *index, int *length);
static inline  void    get_entity(char *buffer, int *index, int *length,entity *entities, int *ent_index, entity *lights, int *light_index);
static inline  void    file_check(char *buffer, int *length);
static inline  void    read(char *buffer, int *length, entity *entities, entity *lights, int *ent_count);
static inline  double  getVecNumber(char *buffer, int *index, int *length, char esc);

// Reads a JSON file into a buffer and removes white space
static inline int read_buffer(FILE *in, char *buffer, int *length) {
  if (!in) {
    fprintf(stderr, "ERROR: File does not exist.\n");
    exit(1);
  }
  char c;
  int i = 0;
  while ((c = getc(in)) != EOF && i < *length) {
    if (isspace(c)) {
      /* character is white space so ignore */
      continue;
    } else {
      /* character is not white space so write to buffer */
      buffer[i] = c;
      i++;
  }
  }
  /* Terminate the buffer */
  buffer[i] = 0;

  return i;
}

// Function will perform initial error checks to ensure
// the file is formatted properly for the program

static inline void file_check(char *buffer, int *length) {

  /* If first character is not '[' after the white space is stripped - ERROR */
  if (buffer[0] != '[') {
    fprintf(stderr, "ERROR: Expected '[' in beginning of file.\n");
    exit(1);
  }

  /* If the buffer is empty avoid executing more than necessary */
  if (buffer[0] == 0 || (buffer[0] == '[' && buffer[1] == ']')) {
    fprintf(stdout, "File is empty. Program will create an empty file.\n");
  }

  if (buffer[1] != '{') {
    fprintf(stderr, "ERROR: Unexpected character reached in file '%c',"
                    " before ']' OR '{'.\n", buffer[1]);
    exit(1);
  }

  /* If the JSON closing character ']', is not met - ERROR */
  if (buffer[((*length) - 1)] != ']') {
    fprintf(stderr, "ERROR: End of file reached before ']'.\n");
    exit(1);
  }

  /* Verify every brace closes */
  int i = 0;
  int j = 0;
  while (i < *length) {
    /* If there is an entity brace, execute until the close is reached */
    if (buffer[i] == '{') {
      j = i;
      while (j < *length) {
        j++;
        if (buffer[j] == '}') {
          i = j;
          break;
        } else if (buffer[j] == '{') {
          /* Possible user typo to be aware of */
          fprintf(stderr, "ERROR: A second '{' reached before '}'.\n");
          exit(1);
        }
      }
      if (buffer[j] != '}') {
        /* If entity is never closed properly */
        fprintf(stderr, "ERROR: End of file reached before '}'.\n");
        exit(1);
      }
    }
    i++;
  }
}

// Writes the entities into memory

static inline void read(char *buffer, int *length, entity *entities,
                             entity *lights, int *ent_count) {

  /* Check the JSON structure */
  file_check(buffer, length);

  /* Get the entities */
  int i = 1, j = 0, k = 0;
  while (i < *length) {
    /* If the entity count exceeds the entity limit - ERROR */
    if ((j + k) == 128) {
      fprintf(stderr, "ERROR: Too many entities in file.\n");
      exit(1);
    }

    if ((i + 1) == *length) {
      break;
    }

    /* Get entity, read until next closing } */
    get_entity(buffer, &i, length, entities, &j, lights, &k);

    /* If we reach some character that doesn't separate an entity - ERROR */
    if (buffer[i] != ',' && buffer[i] != '}') {
      fprintf(stderr, "ERROR: Expected ',', before next '{'.\n");
      exit(1);
    }

    i++;
  }

  /* Return the count of entities and lights */
  ent_count[0] = j + 1;
  ent_count[1] = k;
}

 // Interprets the type of entity from the file and reads into memory

 static inline void get_entity(char *buffer, int *index, int *length,
      entity *entities, int *ent_index, entity *lights,
      int *light_index) {

          if (buffer[*index + 1] != ',' && buffer[*index] != ',' && buffer[*index] != '{') {
            fprintf(stderr, "ERROR: Invalid character reached before next object.\n");
            exit(1);
          }

          /* Check that the first field of the object is its type */
          char *string;
          string = get_string(buffer, index, length);
          /* If first string isn't the object type - ERROR */
          if (strcmp(string, "type") != 0) {
            fprintf(stderr, "ERROR: Expected type as first field of object.\n");
            exit(1);
          }

          string = get_string(buffer, index, length);
          int i = 0; // object and light count
          int state = 0; // state control for light type
          /* Determine correct object, and loop for its properties */
          switch(get_entity_type(string)) {
            /* camera */
            case 1:
              entities[0].type = 1;
              while (i < 2 && (buffer[*index] == ',' || buffer[*index + 1] == ',')) {
                string = get_string(buffer, index, length);
                if (strcmp(string, "width") == 0) {
                  entities[0].camera.width = get_num(buffer, index, length);
                } else if (strcmp(string, "height") == 0) {
                  entities[0].camera.height = get_num(buffer, index, length);
                } else {
                  fprintf(stderr, "ERROR: Missing width or height for camera.\n");
                  exit(1);
                }
                i++;
              }
              if (i != 2) {
                fprintf(stderr, "ERROR: Missing width or height for camera.\n");
                exit(1);
              }
              break;
            /* sphere */
            case 2:
              /* Index 1 is reserved for the camera, so increment accordingly */
              if (*ent_index == 0) {
                (*ent_index)++;
              }
              entities[*ent_index].type = 2;
              while (i < 4 && (buffer[*index] == ',' || buffer[*index + 1] == ',')) {
                string = get_string(buffer, index, length);
                if (strcmp(string, "diffusecolor") == 0) {
                  entities[*ent_index].sphere.diffuse_color = get_vect(buffer, index, length);
              } else if (strcmp(string, "specularcolor") == 0) {
                  entities[*ent_index].sphere.specular_color = get_vect(buffer, index, length);
                } else if (strcmp(string, "position") == 0) {
                  entities[*ent_index].sphere.position = get_vect(buffer, index, length);
                } else if (strcmp(string, "radius") == 0) {
                  entities[*ent_index].sphere.radius = get_num(buffer, index, length);
                } else {
                  fprintf(stderr, "ERROR: Missing color, center, or"
                                  " radius for sphere.\n");
                  exit(1);
                }
                i++;
              }
              if (i != 4) {
                fprintf(stderr, "ERROR: Missing color, center, or"
                                " radius for sphere.\n");
                exit(1);
              }
              (*ent_index)++;
              break;
            /* plane */
            case 3:
              /* Index 1 is reserved for the camera, so increment accordingly */
              if (*ent_index == 0) {
                (*ent_index)++;
              }
              entities[*ent_index].type = 3;
              while (i < 4 && (buffer[*index] == ',' || buffer[*index + 1] == ',')) {
                string = get_string(buffer, index, length);
                if (strcmp(string, "specularcolor") == 0) {
                  entities[*ent_index].plane.specular_color = get_vect(buffer, index, length);
              } else if (strcmp(string, "diffusecolor") == 0) {
                  entities[*ent_index].plane.diffuse_color = get_vect(buffer, index, length);
                } else if (strcmp(string, "position") == 0) {
                  entities[*ent_index].plane.position = get_vect(buffer, index, length);
                } else if (strcmp(string, "normal") == 0) {
                  entities[*ent_index].plane.normal = get_vect(buffer, index, length);
                } else {
                  fprintf(stderr, "ERROR: Missing color, normal, or"
                                  " position for plane.\n");
                  exit(1);
                }
                i++;
              }
              if (i != 4) {
                fprintf(stderr, "ERROR: Missing color, normal, or"
                                " position for plane.\n");
                exit(1);
              }
              break;

            case 4: {
             i = 0;
             while (i < 5 && (buffer[*index] == ',' || buffer[*index + 1] == ',')) {
               string = get_string(buffer, index, length);
               if (strcmp(string, "color") == 0) {
                 lights[*light_index].light.color = get_vect(buffer, index, length);
               } else if (strcmp(string, "position") == 0) {
                 lights[*light_index].light.position = get_vect(buffer, index, length);
               } else if (strcmp(string, "radial-a0") == 0) {
                 lights[*light_index].light.rad_a0 = get_num(buffer, index, length);
               } else if (strcmp(string, "radial-a1") == 0) {
                 lights[*light_index].light.rad_a1 = get_num(buffer, index, length);
               } else if (strcmp(string, "radial-a2") == 0) {
                 lights[*light_index].light.rad_a2 = get_num(buffer, index, length);
               } else if (strcmp(string, "direction") == 0) {
                 state = 1;
                 lights[*light_index].light.spotlight.direction = get_vect(buffer, index, length);
               } else if (strcmp(string, "theta")) {
                 state = 1;
                 lights[*light_index].light.spotlight.theta = get_num(buffer, index, length);
               } else if (strcmp(string, "angular-a0")) {
                 state = 1;
                 lights[*light_index].light.spotlight.ang_a0 = get_num(buffer, index, length);
               } else {
                 fprintf(stderr, "ERROR: Missing color, radial, or"
                                 " position for light.\n");
                 exit(1);
               }
               i++;
             }

             if (i != 5) {
               fprintf(stderr, "ERROR: Missing color, radial, or"
                               " position for light.\n");
               exit(1);
             }

             /* Light is a spotlight so adjust accordingly */
             if (state == 1 && (buffer[*index] == ',' || buffer[*index + 1] == ',')) {
               lights[*light_index].type = 5;
               i = 0;
               while (i < 3 && (buffer[*index] == ',' || buffer[*index + 1] == ',')) {
                 string = get_string(buffer, index, length);
                 if (strcmp(string, "color") == 0) {
                   lights[*light_index].light.color = get_vect(buffer, index, length);
                 } else if (strcmp(string, "position") == 0) {
                   lights[*light_index].light.position = get_vect(buffer, index, length);
                 } else if (strcmp(string, "radial-a0") == 0) {
                   lights[*light_index].light.rad_a0 = get_num(buffer, index, length);
                 } else if (strcmp(string, "radial-a1") == 0) {
                   lights[*light_index].light.rad_a1 = get_num(buffer, index, length);
                 } else if (strcmp(string, "radial-a2") == 0) {
                   lights[*light_index].light.rad_a2 = get_num(buffer, index, length);
                 } else if (strcmp(string, "direction") == 0) {
                   lights[*light_index].light.spotlight.direction = get_vect(buffer, index, length);
                 } else if (strcmp(string, "theta")) {
                   lights[*light_index].light.spotlight.theta = get_num(buffer, index, length);
                 } else if (strcmp(string, "angular-a0")) {
                   lights[*light_index].light.spotlight.ang_a0 = get_num(buffer, index, length);
                 } else {
                   fprintf(stderr, "ERROR: Missing color, radial, direction, theta,"
                                   " or position for light.\n");
                   exit(1);
                 }
                 i++;
               }
               if (i != 3) {
                 fprintf(stderr, "ERROR: Missing color, radial, direction, theta,"
                                 " or position for light.\n");
                 exit(1);
               }
             }

             if (!state) {
               lights[*light_index].type = 4;
             }

             (*light_index)++;
             break;
            }
            default:
              fprintf(stderr, "ERROR: Invalid object type provided."
                      " Ensure that the entities are in lowercase letters.\n");
              exit(1);
          }
          /* Ignore any other characters until the end of the object is reached,
             this is a forgiving part of the parser, but it is also fairly dynamic */
          while (1) {
            if (buffer[*index] == '}') {
              break;
            }
            (*index)++;
          }
        }
// Interprets a string from the file into memory

static inline char *get_string(char *buffer, int *index, int *length) {
  /* 20 characters to avoid any segmentation faults*/
  char *string = malloc(sizeof(char) * 20);
  int i = 0;
  int qCount = 0; /** Counts the number of quotes to be used as an exit tool */
  while (qCount != 2 && (*index) < (*length) && i < 20) {

    /* If we see a quote increment the counter */
    if (buffer[*index] == '"') {
      qCount++;
    }
    /* If current character is a letter add it to the output string */
    if (isalpha(buffer[(*index)]) || isdigit(buffer[(*index)])
        || buffer[*index] == '-') {
      string[i] = buffer[(*index)];
      i++;
    }
    (*index)++;
  }

  if ((*index) == (*length) && qCount != 2) {
    fprintf(stderr, "ERROR: String for entity field not complete before"
                    " end of file reached.\n");
    exit(1);
  }

  if (i == 19 && qCount != 2) {
    fprintf(stderr, "ERROR: String for entity field not complete before"
                    " maximum string size reached.\n");
    exit(1);
  }

  /* If next character after the string is valid for JSON */
  if (buffer[(*index)] == ':' || buffer[(*index)] == ',') {
    string[i] = 0;
    return string;
  }

  /* Unsupported character met at end of string - ERROR */
  fprintf(stderr, "ERROR: Expected ':' OR ',' after string in"
                  " entity for file.\n");
  exit(1);
}

// Interprets a vector from the input file into memory

static inline double *get_vect(char *buffer, int *index, int *length) {
  /* Malloc a double of length 3 double - 3 dimensions */
  double *vector = (double *) malloc(3 * sizeof(double));
  char esc = ',';
  vector[0] = getVecNumber(buffer, index, length, esc);
  esc = ',';
  vector[1] = getVecNumber(buffer, index, length, esc);
  esc = ']';
  vector[2] = getVecNumber(buffer, index, length, esc);

  return vector;
}

// Interprets a number from the character buffer to memory

static inline double get_num(char *buffer, int *index, int *length) {
  /* Value will hold string of a number to be converted */
  char *value = (char *) malloc(100 * sizeof(char));
  int i = 0;
  int success = 0;

  /* While the number hasn't terminated */
  while ((*index < *length) && i < 100) {

    /* See if the end of the number has been reached */
    if (buffer[*index] == ',' || buffer[*index] == '}') {
      success = 1;
      break;
    }

    /* If next character is not valid - ERROR */
    if (!isdigit(buffer[*index]) && buffer[*index] != '.'
        && buffer[*index] != ':' && buffer[*index] != '-') {
      fprintf(stderr, "ERROR: Expected valid number in entity for file.\n");
      exit(1);
    }

    /* Otherwise add current character to number buffer */
    if (isdigit(buffer[*index]) || buffer[*index] == '.'
        || buffer[*index] == '-') {
      value[i] = buffer[*index];
      i++;
    }

    (*index)++;
  }

  /* End of buffer reached before valid closing character reached */
  if (!success) {
    fprintf(stderr, "ERROR: End of buffer reached before valid closing"
                    " character reached for number.\n");
    exit(1);
  }

  /* Convert string to number and return it */
  return atof(value);
}

// Determines the type of entity that is being presented
static inline int get_entity_type(char *string) {
  if (strcmp(string, "camera") == 0) {
    return 1;
  } else if (strcmp(string, "sphere") == 0) {
    return 2;
} else if (strcmp(string, "plane") == 0) {
    return 3;
  } else if (strcmp(string, "light") == 0) {
    return 4;
  } else {
    fprintf(stderr, "ERROR: Invalid type supplied for entity.\n");
    exit(1);
  }
}

static inline double getVecNumber(char *buffer, int *index, int *length, char esc) {
  /* Value will hold string of a number to be converted */
  char *value = (char *) malloc(100 * sizeof(char));
  int i = 0;
  int success = 0;
  if (esc == ',' && buffer[*index] == ',') {
    (*index)++;
  }

  /* While the number hasn't terminated */
  while (*index < *length && i < 100) {

    /* See if the end of the number has been reached */
    if (buffer[*index] == esc) {
      success = 1;
      break;
    }

    /* Otherwise add current character to number buffer */
    if (isdigit(buffer[*index]) || buffer[*index] == '.'
        || buffer[*index] == '-') {
      value[i] = buffer[*index];
      i++;
    }

    /* If next character is not valid - ERROR */
    if (!isdigit(buffer[*index]) && buffer[*index] != '.'
        && buffer[*index] != ':' && buffer[*index] != '-'
        && buffer[*index] != '[' && buffer[*index] != ','
        && buffer[*index] != ']') {
      fprintf(stderr, "ERROR: Expected valid number in entity for file.\n");
      exit(1);
    }

    (*index)++;
  }

  /* End of buffer reached before valid closing character reached */
  if (!success) {
    fprintf(stderr, "ERROR: End of buffer reached before valid closing"
                    " character reached for number.\n");
    exit(1);
  }

  /* Convert string to number and return it */
  return atof(value);
}
