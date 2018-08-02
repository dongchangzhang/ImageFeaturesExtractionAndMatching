FLAG=-std=c++11 
LIBS=`pkg-config --cflags --libs opencv` 
all: main drawPoints drawLine

main:main.o feature.o
	g++ $(FLAG)  main.o feature.o -o main $(LIBS)

drawPoints:points.o feature.o
	g++ $(FLAG) feature.o points.o -o drawPoints $(LIBS) 

drawLine:line.o feature.o
	g++ $(FLAG) feature.o line.o -o drawLine $(LIBS) 

feature.o:feature.cpp feature.h
	g++ $(FLAG) -c $< -o $@ $(LIBS)

main.o:main.cpp feature.h
	g++ $(FLAG) -c $< -o $@ $(LIBS)

points.o:drawPoints.cpp feature.h
	g++ $(FLAG) -c $< -o $@ $(LIBS)

line.o:drawLine.cpp feature.h
	g++ $(FLAG) -c $< -o $@ $(LIBS)


clean:
	rm -rf *.o
	rm -rf main
	rm -rf drawLine
	rm -rf drawPoints
	rm -rf *.txt

