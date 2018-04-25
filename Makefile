all: main draw

main:main.o feature.o
	g++ `pkg-config --cflags --libs opencv` -o main main.o feature.o

feature.o:feature.cpp feature.h
	g++ `pkg-config --cflags --libs opencv` -c $< -o $@

main.o:main.cpp feature.h
	g++ `pkg-config --cflags --libs opencv` -c $< -o $@

clean:
	rm -rf *.o
	rm -rf main
	rm -rf draw
	rm -rf *.txt
draw:
	g++ `pkg-config --cflags --libs opencv`  draw.cpp -o draw
