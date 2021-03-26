CC = g++
CFLAGS = -g -Wall -lOpenGL -lGLU -lglfw -lglad -lSOIL -lfreeimage -lassimp
OBJ = main.o shader.o camera.o

nbody: $(OBJ)
	$(CC) $(CFLAGS) -o nbody $(OBJ)

main.o: main.cpp
	$(CC) -g -c main.cpp
shader.o: shader.cpp shader.h
	$(CC) -g -c shader.cpp
camera.o: camera.cpp camera.h
	$(CC) -g -c camera.cpp

clean:
	rm -f *.o nbody
