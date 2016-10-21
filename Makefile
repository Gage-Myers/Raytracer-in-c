all:
	gcc raycast.c -o raytrace -lm
run:
	./raytrace 128 128 in.json out.ppm
clean:
	rm raycast
	clear
