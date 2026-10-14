#include "cudaKernel.h"

// dist[] and bucketOf[] are updated by other threads (with atomics) while a kernel runs.
// Reading them through a volatile pointer forces a real memory access, so a thread never works
// with a value that another thread has already lowered.
__device__ inline int readFresh(const int *p)
{
    return *((const volatile int *)p);
}

// cuda kernel to initialize distance array with 2147483647
__global__ void distanceInitialization(int *dist, int *bucketOf, int *settledIn, int n, int root)
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < n)
    {
        dist[index] = INF_DIST;
        bucketOf[index] = INF_DIST;
        settledIn[index] = -1;
        if (index == root)
        {
            dist[index] = 0;
            bucketOf[index] = 0;
        }
    }
}

// relaxes the edge (u -> v) with new distance nd. atomicMin keeps the smallest value when several
// threads reach the same node; the node is then (re)inserted into the bucket of its new distance.
__device__ inline void relax(int *dist, int *bucketOf, int v, int nd, int delta)
{
    if (nd < readFresh(&dist[v]))
    {
        int old = atomicMin(&dist[v], nd);
        if (nd < old)
        {
            // the new distance must be visible to other threads before v appears in its bucket
            __threadfence();
            atomicMin(&bucketOf[v], nd / delta);
        }
    }
}

// light edges relaxation kernel
//  n --> number of nodes, buc --> current bucket number, delta --> bucket width
//  changed --> set to 1 if at least one node of the bucket was processed in this launch
__global__ void lightEdgesKernel(int *dist, int *bucketOf, int *settledIn, const int *row_ptr, const int *light_end,
                                 const int *col_ind, const int *weight, int n, int buc, int delta, int *changed)
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    // checking if the index is within the bucket
    if (index < n && readFresh(&bucketOf[index]) == buc)
    {
        int u = index;
        // take the node out of the bucket BEFORE reading its distance: if another thread lowers
        // dist[u] after this point it also puts u back into the bucket, so no update is lost.
        atomicExch(&bucketOf[u], INF_DIST);
        __threadfence();
        settledIn[u] = buc;
        int u_dist = readFresh(&dist[u]);
        // exploring neighbours through light edges only
        for (int i = row_ptr[u]; i < light_end[u]; i++)
        {
            relax(dist, bucketOf, col_ind[i], u_dist + weight[i], delta);
        }
        *changed = 1;
    }
}

// heavy edges relaxation kernel: every node whose light edges were relaxed in bucket buc now has
// its final distance, so its heavy edges (weight > delta) are relaxed exactly once.
__global__ void heavyEdgesKernel(int *dist, int *bucketOf, const int *settledIn, const int *row_ptr, const int *light_end,
                                 const int *col_ind, const int *weight, int n, int buc, int delta)
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < n && settledIn[index] == buc)
    {
        int u = index;
        int u_dist = readFresh(&dist[u]);
        for (int i = light_end[u]; i < row_ptr[u + 1]; i++)
        {
            relax(dist, bucketOf, col_ind[i], u_dist + weight[i], delta);
        }
    }
}

// finds the smallest non-empty bucket, so that empty buckets are skipped in one launch
__global__ void nextBucketKernel(const int *bucketOf, int n, int *nextBucket)
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < n)
    {
        int b = readFresh(&bucketOf[index]);
        if (b < readFresh(nextBucket))
        {
            atomicMin(nextBucket, b);
        }
    }
}
