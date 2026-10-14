#ifndef DISTRIBUTIONCALC_H
#define DISTRIBUTIONCALC_H
#include <vector>

// Splits n elements over "size" processes.
//   gpuRanks  = 0 : CPU-only, equal shares
//   gpuRanks  = 1 : the last process sorts on the GPU. Its share is gpuWeight times the share of
//                   one CPU process, but never more than gpuCapacity elements (what fits on the GPU).
void distriCalc(long n, int size, int gpuRanks, double gpuWeight, long gpuCapacity,
                std::vector<long> &distributionArray, std::vector<long> &displacementarray);

#endif
