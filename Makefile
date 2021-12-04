SHELL = /bin/bash
CC := /usr/bin/gcc
CFLAGS = -O2 -Wall
DEBUG_CFLAGS = -g -Wall
LIBS = -L/usr/include/SDL2 -lSDL2main -lSDL2 -ldl -lm 
MAIN_FILES = main.c mathlib.c sound.c

CUR_PATH := $(shell dirname $(realpath $(firstword ${MAKEFILE_LIST})))

whgen:
	pushd ${CUR_PATH}/src \
		&& $(CC) $(MAIN_FILES) -o whgen $(CFLAGS) $(LIBS) \
		&& mv whgen .. \
		&& popd

debug:
	pushd ${CUR_PATH}/src \
		&& $(CC) $(MAIN_FILES) -o whgen $(DEBUG_CFLAGS) $(LIBS) \
		&& mv whgen .. \
		&& popd

plots:
	python ./scripts/plot_data.py

.PHONY: clean whgen plots

clean:
	rm *.o tmp whgen
