SRC = src/*.c
OUT = PACK.EXE

all:
	gcc -o $(OUT) $(SRC) -std=c99 -static -lraylib -lopengl32 -lgdi32

run: all
	./PACK.EXE
