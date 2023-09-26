extrapolator: lodepng.o depthExtrap.o
	g++ -std=c++11 -pthread -o ./depthExtrap ./depthExtrap.o ./lodepng/lodepng.o

depthExtrap.o: depthExtrap.cpp
	g++ -std=c++11 -pthread -c ./depthExtrap.cpp -o ./depthExtrap.o -DLINUX_BUILD

lodepng.o: ./lodepng/lodepng.cpp ./lodepng/lodepng.h
	g++ -std=c++11 -pthread -c ./lodepng/lodepng.cpp -o ./lodepng/lodepng.o

clean:
	rm *.o
