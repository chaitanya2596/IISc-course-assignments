CXX      = g++
CXXFLAGS = -std=c++11 -O3 -fopenmp

all: histogram inputGenerator

histogram: src/main.cpp src/histogram.cpp src/histogram.h
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp src/histogram.cpp

inputGenerator: src/inputGenerator.cpp
	$(CXX) $(CXXFLAGS) -o $@ src/inputGenerator.cpp

clean:
	rm -f histogram inputGenerator
