#include <iostream>
#include <cstdlib>
#include <chrono>
#include <cuda_runtime.h>

#include "mergeGpu.h"

using namespace std;

#define CUDA_CHECK(call)                                                                       \
    {                                                                                          \
        cudaError_t e = (call);                                                                \
        if (e != cudaSuccess)                                                                  \
        {                                                                                      \
            cerr << "CUDA error: " << cudaGetErrorString(e) << " at line " << __LINE__ << endl; \
            exit(1);                                                                           \
        }                                                                                      \
    }

// Every thread produces SEGMENT consecutive elements of the output of one merge pass.
#define SEGMENT 1024

__device__ inline long lmin(long a, long b) { return a < b ? a : b; }
__device__ inline long lmax(long a, long b) { return a > b ? a : b; }

// writes the next "count" elements of the merge of A[i..na) and B[j..nb) to dest
__device__ void gpu_merge(const int *A, long na, const int *B, long nb, long i, long j, int *dest, long count)
{
    for (long k = 0; k < count; k++)
    {
        if (i < na && (j >= nb || A[i] <= B[j]))
        {
            dest[k] = A[i++];
        }
        else
        {
            dest[k] = B[j++];
        }
    }
}

// One pass of bottom-up merge sort: sorted runs of length "width" in source are merged in pairs
// into runs of length 2*width in dest.
//
// The OUTPUT is cut into pieces of SEGMENT elements and each thread fills one piece, so that all
// threads stay busy even in the last passes, where there are only a few very long runs:
//   - if a piece contains whole pairs (2*width <= SEGMENT) the thread merges them one after another
//   - otherwise the piece lies inside one pair. A binary search ("merge path" / co-rank) finds how
//     many elements of the left and of the right run come before the piece, and the thread merges
//     from there.
__global__ void gpu_mergesort(const int *source, int *dest, long size, long width)
{
    long idx = (long)blockDim.x * blockIdx.x + threadIdx.x;
    long start = idx * SEGMENT;
    if (start >= size)
        return;
    long end = lmin(start + SEGMENT, size);

    if (2 * width <= SEGMENT)
    {
        for (long p = start; p < end; p += 2 * width)
        {
            long middle = lmin(p + width, size);
            long pend = lmin(p + 2 * width, size);
            gpu_merge(source + p, middle - p, source + middle, pend - middle, 0, 0, dest + p, pend - p);
        }
    }
    else
    {
        long p = (start / (2 * width)) * (2 * width); // first element of the pair this piece belongs to
        long middle = lmin(p + width, size);
        long pend = lmin(p + 2 * width, size);
        const int *A = source + p;
        const int *B = source + middle;
        long na = middle - p, nb = pend - middle;

        // co-rank: of the first k outputs of the merge, i come from A and k - i from B
        long k = start - p;
        long lo = lmax(0, k - nb), hi = lmin(k, na);
        while (lo < hi)
        {
            long mid = (lo + hi) / 2;
            if (A[mid] <= B[k - mid - 1])
                lo = mid + 1;
            else
                hi = mid;
        }
        gpu_merge(A, na, B, nb, lo, k - lo, dest + start, end - start);
    }
}

double gpuMergeSort(int *data, long size)
{
    auto t0 = chrono::high_resolution_clock::now();
    int *D_data, *D_swp;

    CUDA_CHECK(cudaMalloc((void **)&D_data, size * sizeof(int)));
    CUDA_CHECK(cudaMalloc((void **)&D_swp, size * sizeof(int)));
    CUDA_CHECK(cudaMemcpy(D_data, data, size * sizeof(int), cudaMemcpyHostToDevice));

    int *A = D_data;
    int *B = D_swp;

    int threadsPerBlock = 256;
    long nThreads = (size + SEGMENT - 1) / SEGMENT;
    int blocksPerGrid = (int)((nThreads + threadsPerBlock - 1) / threadsPerBlock);

    for (long width = 1; width < size; width <<= 1)
    {
        gpu_mergesort<<<blocksPerGrid, threadsPerBlock>>>(A, B, size, width);
        std::swap(A, B);
    }
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());

    CUDA_CHECK(cudaMemcpy(data, A, size * sizeof(int), cudaMemcpyDeviceToHost));

    cudaFree(D_data);
    cudaFree(D_swp);
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::microseconds>(t1 - t0).count() / 1e6;
}

void gpuMemory(long *freeBytes, long *totalBytes)
{
    size_t f, t;
    CUDA_CHECK(cudaMemGetInfo(&f, &t));
    *freeBytes = (long)f;
    *totalBytes = (long)t;
}
