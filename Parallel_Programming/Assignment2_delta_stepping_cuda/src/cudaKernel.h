#ifndef CUDAKERNEL_H
#define CUDAKERNEL_H
#include <cuda_runtime.h>

#define INF_DIST 2147483647

// The per-node data is kept in three separate int arrays (structure of arrays) instead of an
// array of pairs, so that neighbouring threads read neighbouring 4-byte values (coalesced access):
//   dist[v]      tentative distance of v
//   bucketOf[v]  bucket in which v is waiting to be relaxed, INF_DIST if it is not waiting
//   settledIn[v] last bucket in which the light edges of v were relaxed (-1 if never)

__global__ void distanceInitialization(int *dist, int *bucketOf, int *settledIn, int n, int root);

__global__ void lightEdgesKernel(int *dist, int *bucketOf, int *settledIn, const int *row_ptr, const int *light_end,
                                 const int *col_ind, const int *weight, int n, int buc, int delta, int *changed);

__global__ void heavyEdgesKernel(int *dist, int *bucketOf, const int *settledIn, const int *row_ptr, const int *light_end,
                                 const int *col_ind, const int *weight, int n, int buc, int delta);

__global__ void nextBucketKernel(const int *bucketOf, int n, int *nextBucket);

#endif
