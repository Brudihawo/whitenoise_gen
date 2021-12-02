CC := /usr/bin/gcc
MAKEFLAGS := -Wall

whgen:
	$(CC) main.c -o whgen $(MAKEFLAGS) -L/usr/include/SDL2 -lSDL2main -lSDL2

.PHONY: clean

clean:
	rm *.o tmp whgen
