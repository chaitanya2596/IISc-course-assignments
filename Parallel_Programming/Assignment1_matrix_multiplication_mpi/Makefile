MPICXX   = mpic++
CXX      = g++
CXXFLAGS = -std=c++11 -O3

all: cannon summa seq_matmul Mat_generator

cannon: src/cannon.cpp src/common.cpp src/common.h
	$(MPICXX) $(CXXFLAGS) -o $@ src/cannon.cpp src/common.cpp

summa: src/summa.cpp src/common.cpp src/common.h
	$(MPICXX) $(CXXFLAGS) -o $@ src/summa.cpp src/common.cpp

seq_matmul: src/seq_matmul.cpp src/common.h
	$(MPICXX) $(CXXFLAGS) -o $@ src/seq_matmul.cpp

Mat_generator: src/Mat_generator.cpp
	$(CXX) $(CXXFLAGS) -o $@ src/Mat_generator.cpp

clean:
	rm -f cannon summa seq_matmul Mat_generator
