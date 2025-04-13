
all:
	gcc -g main.c `sdl2-config --cflags --libs` -lSDL2_ttf -lSDL2_image -lm -Wall

nt:
	gcc -g main.c -I"external/include" -L"external/lib"  -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -Wall -o build/app.exe
 
run:
	./a.out

run-nt:
	./build/app.exe