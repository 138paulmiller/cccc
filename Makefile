CC=gcc
SRC=src
FLAGS = -lGL -lGLEW -lSDL2 -lSDL2main
all:
	$(CC) $(SRC)/ccc.c -o cccc $(FLAGS)

bf:
	$(CC) $(SRC)/ccc.c -DBF=1 -o cccc $(FLAGS)
opengl:
	$(CC) $(SRC)/ccc.c -DOPENGL=1 -o cccc $(FLAGS)
clean:
	rm cccc
