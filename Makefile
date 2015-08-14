CXXFLAGS+=-O3 -std=c++11 -fopenmp
INCLUDES_DIR=
LIBS=-lgomp -lpng -lX11 -lpthread
CXX=g++

test: main
	./main 800 400

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES_DIR) $< -o $@

main: main.o graphics.o
	$(CXX) $^ $(LIBS) -o $@

clean:
	rm *.o main

