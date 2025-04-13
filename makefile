
all:
	gcc -g main.c `sdl2-config --cflags --libs` -lSDL2_ttf -lSDL2_image -lm -Wall
 
run:
	./a.out