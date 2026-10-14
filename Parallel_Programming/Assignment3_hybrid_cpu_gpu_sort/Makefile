# Makefile for compiling MPI + CUDA code
# GPU build needs:   module load gnu12/12.4.0 openmpi4/4.1.6 CUDA/12.8   (or your cluster's equivalents)
MPICXX   = mpic++
NVCC     = nvcc
CXXFLAGS = -std=c++11 -O3
CUDA_LIB = $(shell dirname $(shell which $(NVCC) 2>/dev/null) 2>/dev/null)/../lib64
COMMON   = src/distributionCalc.cpp src/mergeAlgo.cpp

all: hybrid_sort

# CPU + GPU executable (modes: cpu, hybrid)
hybrid_sort: src/main.cpp $(COMMON) src/mergeGpu.cu src/mergeGpu.h src/mergeAlgo.h src/distributionCalc.h
	$(NVCC) -O3 -c src/mergeGpu.cu -o mergeGpu.o
	$(MPICXX) $(CXXFLAGS) -DUSE_GPU -o $@ src/main.cpp $(COMMON) mergeGpu.o -L$(CUDA_LIB) -lcudart -Wl,-rpath,$(CUDA_LIB)

# CPU-only executable (mode: cpu), builds without CUDA
cpu_sort: src/main.cpp $(COMMON) src/mergeAlgo.h src/distributionCalc.h
	$(MPICXX) $(CXXFLAGS) -o $@ src/main.cpp $(COMMON)

clean:
	rm -f hybrid_sort cpu_sort mergeGpu.o
