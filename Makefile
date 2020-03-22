
# A very NAIVE Makefile

CFLAGS=-Iinclude/ -std=c++11 -Wall -pedantic -Werror
LDFLAGS=-lglfw -ldl -lmpv

all:
	mkdir -p build/
	g++ glad/glad.c ${CFLAGS} -c -o build/glad.o
	g++ mpvideocube.cpp ${CFLAGS} -c -o build/mpvideocube.o
	g++ -o mpvideocube build/*.o ${LDFLAGS}


clean:
	rm -f build/*.o
	rm -f mpvideocube
