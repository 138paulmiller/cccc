CC=gcc
SRC=src
FLAGS = -lGL -lGLEW -lSDL2 -lSDL2main -march=native -O3
COMPILE = $(CC) $(SRC)/ccc.c -o cccc $(FLAGS)


bf:
	$(COMPILE) -DBF=1
gl:
	$(COMPILE) -DOPENGL=1


debug-bf:
	$(COMPILE) -DDEBUG=1 -DBF=1 
debug-gl:
	$(COMPILE) -DDEBUG=1 -DOPENGL=1 

clean:
	rm cccc
