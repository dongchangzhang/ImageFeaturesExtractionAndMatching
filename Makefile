all: main drawPoints drawLine

main:main.o feature.o
	g++ -std=c++11 `pkg-config --cflags --libs opencv` -std=c++11  -o main main.o feature.o

feature.o:feature.cpp feature.h
	g++ -std=c++11 `pkg-config --cflags --libs opencv` -std=c++11 -c $< -o $@

main.o:main.cpp feature.h
	g++ -std=c++11 `pkg-config --cflags --libs opencv` -std=c++11 -c $< -o $@

clean:
	rm -rf *.o
	rm -rf main
	rm -rf drawLine
	rm -rf drawPoints
	rm -rf *.txt

drawPoints:
	g++ `pkg-config --cflags --libs opencv` -std=c++11 feature.o drawPoints.cpp -o drawPoints

drawLine:
	g++ `pkg-config --cflags --libs opencv` -std=c++11 feature.o drawLine.cpp -o drawLine 


