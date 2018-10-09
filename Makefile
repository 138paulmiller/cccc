CC=gcc
FLAGS = -lGL -lGLEW -lSDL2 -lSDL2main
all:
	$(CC) ccc.c -o cccc $(FLAGS)
