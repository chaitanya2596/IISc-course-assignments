# GPU build needs the CUDA compiler:   module load gnu12/12.4.0 CUDA/12.8
NVCC     = nvcc
CXX      = g++
CXXFLAGS = -std=c++11 -O3 -w
COMMON   = src/readMTXFile.cpp src/adjListToCSR.cpp src/seqDeltaStepping.cpp

all: sssp sssp_seq

# sequential + Dijkstra + GPU delta-stepping in one program
sssp: $(COMMON) src/main.cu src/cudaKernel.cu src/cudaKernel.h src/graph.h
	$(NVCC) $(CXXFLAGS) -o $@ src/main.cu src/cudaKernel.cu $(COMMON)

# CPU-only program (sequential delta-stepping checked against Dijkstra)
sssp_seq: $(COMMON) src/seq_main.cpp src/graph.h
	$(CXX) $(CXXFLAGS) -o $@ src/seq_main.cpp $(COMMON)

clean:
	rm -f sssp sssp_seq
