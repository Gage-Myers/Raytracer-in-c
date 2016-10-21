# Raytracer-in-c
A C program that will allow for JSON files to be read in and will produce a ppm image file

# Requirements
gcc and a Unix based command line or MinGW for Windows

Must use -lm at the end of your gcc command as it is the only way to compile math.h

# Usage
make: build project - runs gcc raycast.c -o raytrace -lm

make run: run ./raytrace 128 128 in.json out.ppm

make clean:  remove all unnecessary files

To run with customized inputs enter ./raycast width height json-file output-file
