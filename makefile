
all:
	gcc -g main.c `sdl2-config --cflags --libs` -lSDL2_ttf -lSDL2_image -lm -Wall

nt:
	gcc -g main.c -I"external/include" -L"external/lib"  -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -Wall -o build/app.exe
 
run:
	./a.out

run-nt:
	./build/app.exe

clean:
	rm -rf build/*.exe *.out *.exe

prune: clean
	rm -rf build/ __tmp__/ external/

build-watcher:
	gcc -o watcher.out watcher.c -Wall