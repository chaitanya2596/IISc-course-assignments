MPICXX   = mpic++
CXX      = g++
CXXFLAGS = -std=c++11 -O3

all: mergesort inputGenerator

mergesort: src/main.cpp src/mergeSort.cpp src/mergeSort.h
	$(MPICXX) $(CXXFLAGS) -o $@ src/main.cpp src/mergeSort.cpp

inputGenerator: src/inputGenerator.cpp
	$(CXX) $(CXXFLAGS) -o $@ src/inputGenerator.cpp

clean:
	rm -f mergesort inputGenerator
