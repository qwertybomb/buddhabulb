cc = clang
flags = -s -Wall -Wextra -pedantic -std=c11 -march=native -Ofast -fopenmp
libs = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglew32 -lomp

all: main.c
	@$(cc) $(flags) main.c -o prog $(libs)

clean:
	@rm prog
