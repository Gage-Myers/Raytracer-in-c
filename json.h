#include "entity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_buffer(FILE *in, char *buff, int *len);
int get_form(char *string);
char *find_string(char *buff, int *ind);
double *find_vect(char *buff, int *ind);
double find_num(char *buff, int *ind);
void find_obj(char *buff, int *ind, entity *entities, int *ent_ind);
int error_parse(char *buff, int *len);
int read_scene(char *buff, int *len, entity *entities);

int read_buffer(FILE *in, char *buff, int *len) {
    if (!in) {
        /* File not found */
        fprintf(stderr, "ERROR: The file could not be found.\n");
        return -1;
    }
    char get;
    int i = 0;
    while ((get = getc(in)) != EOF && i < *len) {
        if (isspace(get)) {
            /* whitespace */
            continue;
        } else {
            buff[i] = get;
            i++;
        }
    }
    return 0;
}

int error_parse(char *buff, int *len) {
    int i = 0;

    if (buff[i] != '[') {
        fprintf(stderr, "ERROR: First character should be [\n");
        exit(1);
    }

    if ((buff[i] == '[' && buff[i + 1] == ']') || buff[i] == 0) {
        fprintf(stderr, "ERROR: File has no contents\n");
        exit(1);
    }

    int exit_loop = 0;

    while (i < *len) {

        if (i == 1 && buff[i] != '{') {
            fprintf(stderr, "ERROR: %c is an unexpected character\n",buff[i] );
            exit(1);
        }

        if (buff[i] = ']') {
            exit_loop = 1;
        }
        i++;
    }

    if (!exit_loop) {
        fprintf(stderr, "ERROR: End of file reached without closing JSON file\n");
        exit(1);
    }

    i = 0;
    int j = 0;
    while (i < *len) {

        if (buff[i] == '{') {
            j = i;
            while (j < *len) {
                j++;
                if (buff[j] == '}') {
                    i == j;
                    break;
                } else if(buff[i] == '{') {
                    fprintf(stderr, "ERROR: Second object opened before the first was closed\n");
                    exit(1);
                }
            }
            if (buff[i] != '}') {
                fprintf(stderr, "ERROR: End of object reached without closing entity\n");
                exit(1);
            }
        }
        i++;
    }
    return 1;
}

int read_scene(char *buff, int *len, entity *entities) {
      // Ensure JSON is formatted properly
      if (error_parse(buff, len) == 0) {
        return 1;
      }

      // Variables to hold state for loops and conditions
      int i = 1, j = 1, state = 0;
      while (i  < *len) {
        if (state == 0) {
          // finds the object that is currently eing parsed
          find_obj(buff, &i, entities, &j);
          state = 1;
          continue;
        }
        i++;
        // breaks loop at EOF
        if (buff[i] == ']') {
          break;
        }
        // Checks for comma between items
        if (state && buff[i] != ',') {
          fprintf(stderr, "ERROR: Expected a comma between items'.\n");
          exit(1);
        }
        // Error checking for JSON items
        if (j == 129) {
          fprintf(stderr, "ERROR: Too many entities in file.\n");
          exit(1);
        }
        find_obj(buff, &i, entities, &j);
        j++;
      }
      return j;
  }

void find_obj(char *buff, int *ind,  entity *entities, int *ent_ind) {
  char *string;
  string = find_string(buff, ind);
  // Checks to make sure first field is type
  if (strcmp(string, "type") != 0) {
    fprintf(stderr, "ERROR: Expected type as first entry of entity.\n");
    exit(1);
  }
  string = find_string(buff, ind);
  int i = 0;
  /* Determine correct object, and loop for its properties */
  switch(get_form(string)) {
    // camera
    case 1:
      entities[0].type = 1;
      while (i < 2) {
        string = find_string(buff, ind);
        if (strcmp(string, "width") == 0) {
          entities[0].camera.width = find_num(buff, ind);
        } else if (strcmp(string, "height") == 0) {
          entities[0].camera.height = find_num(buff, ind);
        } else {
          fprintf(stderr, "ERROR: Missing width or height for camera.\n");
          exit(1);
        }
        i++;
      }
      break;
    // sphere
    case 2:
      entities[*ent_ind].type = 2;
      while (i < 3) {
        string = find_string(buff, ind);
        if (strcmp(string, "color") == 0) {
          entities[*ent_ind].sphere.color = find_vect(buff, ind);
        } else if (strcmp(string, "position") == 0) {
          entities[*ent_ind].sphere.position = find_vect(buff, ind);
        } else if (strcmp(string, "radius") == 0) {
          entities[*ent_ind].sphere.radius = find_num(buff, ind);
        } else {
          fprintf(stderr, "ERROR: Missing color, center, or radius for sphere.\n");
          exit(1);
        }
        i++;
      }
      break;
    // plane
    case 3:
      entities[*ent_ind].type = 3;
      while (i < 3) {
        string = find_string(buff, ind);
        if (strcmp(string, "color") == 0) {
          entities[*ent_ind].plane.color = find_vect(buff, ind);
        } else if (strcmp(string, "position") == 0) {
          entities[*ent_ind].plane.position = find_vect(buff, ind);
        } else if (strcmp(string, "normal") == 0) {
          entities[*ent_ind].plane.normal = find_vect(buff, ind);
        } else {
          fprintf(stderr, "ERROR: Missing color, normal, or position for plane.\n");
          exit(1);
        }
        i++;
      }
      break;
  }
  /* Ignore any other characters until the end of the object is reached,
     this is a forgiving part of the parser, but it is also fairly dynamic */
  while (buff[*ind] != '}') {
    (*ind)++;
  }
}

char *find_string(char *buff, int *ind) {
  /* 8 characters because 'position' is the longest string possible */
  char *string = malloc(sizeof(char) * 8);
  int i = 0;
  int qCount = 0; /** Counts the number of quotes to be used as an exit tool */
  while (qCount != 2) {
    /* If we see a quote increment the counter */
    if (buff[*ind] == '"') {
      qCount++;
    }
    /* If current character is a letter add it to the output string */
    if (isalpha(buff[(*ind)])) {
      string[i] = buff[(*ind)];
      i++;
    }
    (*ind)++;
  }
  /* If next character after the string is valid for JSON */
  if (buff[(*ind)] == ':' || buff[(*ind)] == ',') {
    string[i] = 0;
    return string;
  }
  /* Unsupported character met - ERROR */
  fprintf(stderr, "ERROR: Expected ':' OR ',' after string in object for file.\n");
  exit(1);
}

double *find_vect(char *buff, int *ind) {
  /* Malloc a double length 3 double - 3 dimensions */
  double *vector = (double *) malloc(3 * sizeof(double));
  int i = 0;
  (*ind)++;
  while (i < 3) {
    (*ind)++;
    vector[i] = find_num(buff, ind);
    i++;
  }
  return vector;
}

double find_num(char *buff, int *ind) {
  /* Value will hold string of a number to be converted */
  char *value = (char *) malloc(100 * sizeof(char));
  int i = 0;
  /* While the number hasn't terminated */
  while (buff[*ind] != ',' && buff[*ind] != '}'
         && buff[*ind] != ']') {
    /* If next character is not valid - ERROR */
    if (!isdigit(buff[*ind]) && buff[*ind] != '.'
        && buff[*ind] != ':' && buff[*ind] != '-') {
      fprintf(stderr, "ERROR: Expected valid number in object for file.\n");
      exit(1);
    }
    /* Otherwise add current character to number buff */
    if (isdigit(buff[*ind]) || buff[*ind] == '.'
        || buff[*ind] == '-') {
      value[i] = buff[*ind];
      i++;
    }
    (*ind)++;
  }
  /* Convert string to number and return it */
  return atof(value);
}

int get_form(char *string) {
  if (strcmp(string, "camera") == 0) {
    return 1;
  } else if (strcmp(string, "sphere") == 0) {
    return 2;
  } else if (strcmp(string, "plane") == 0) {
    return 3;
  } else {
    fprintf(stderr, "ERROR: Invalid type supplied for object.\n");
    exit(1);
  }
}
