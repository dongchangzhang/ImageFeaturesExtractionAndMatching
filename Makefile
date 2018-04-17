all: main

main:utils.o ASiftDetector.o main.o
	g++ `pkg-config --cflags --libs opencv` -o main utils.o ASiftDetector.o main.o

utils.o:utils.cpp utils.h
	g++ `pkg-config --cflags --libs opencv` -c $< -o $@

ASiftDetector.o:ASiftDetector.cpp ASiftDetector.h
	g++ `pkg-config --cflags --libs opencv` -c $< -o $@

main.o:main.cpp ASiftDetector.h utils.h
	g++ `pkg-config --cflags --libs opencv` -c $< -o $@

clean:
	rm -rf *.o
	rm -rf main
