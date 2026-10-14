# Assignment 3: integer sort using the CPU cores and the GPU together (MPI + CUDA)

## Problem

Question sheet: [question.pdf](question.pdf)

This assignment will get you started on the simultaneous use of both the CPU and GPU cores for computations.

The problem is of integer sort. Take a large array such that the size of the array is four times
more than what can be accommodated in the GPU memory of the platform you will be using. Hence the
array has to be divided between CPU and GPU. Use both the CPU and GPU executions simultaneously to
sort their respective parts of the arrays, and finally merge sort them to produce the final sorted
output for the entire array.

Compare the execution times of 1) CPU-only executions and 2) Hybrid CPU-GPU executions. Also, show
CUDA profiling results.

## What is implemented

`mpirun -np P ./hybrid_sort <cpu|hybrid> <n> [gpuCapacity] [gpuWeight]`

1. **Split.** In `cpu` mode the P processes get equal parts. In `hybrid` mode the last process sorts
   on the GPU and the other P-1 on CPU cores. The GPU process gets `gpuWeight` times the part of one
   CPU process, but never more than `gpuCapacity` integers (default: what fits in the free GPU memory) (`src/distributionCalc.cpp`).
2. **Local sort, all processes at the same time.** CPU processes use a merge sort with one scratch
   buffer (`src/mergeAlgo.cpp`). The GPU process runs a bottom-up merge sort in CUDA (`src/mergeGpu.cu`).
3. **Parallel merge.** The processes merge in pairs, the survivors merge in pairs again, and so on,
   until the whole sorted array is in process 0 after log2(P) rounds. The merges of one round run at the same time on different processes.
4. **Check.** Process 0 verifies that the result is in order and holds the same elements as the input (sum and xor of all values).

The input is generated inside the program: element i gets a fixed pseudo-random value, so the same
array is produced for any number of processes.

Pseudocode of the MPI part:

```
every process p:   my part = elements [start(p), start(p) + count(p))
                   sort my part            (CPU merge sort, or GPU merge sort on the last process)
for step = 1, 2, 4, ... < P:
    if p is a multiple of 2*step:  receive the sorted run of process p + step, merge it with mine
    else:                          send my sorted run to process p - step and stop
process 0 holds the sorted array
```

## Optimizations

1. **Parallel merge tree:** the merges of each round run on different processes at the same time, so
   process 0 is not the only one merging.
2. **GPU merge sort that keeps every thread busy in every pass:** the output of each pass is cut into
   pieces of 1024 elements, one per thread, and a binary search (the "merge path" method) tells each
   thread where its piece starts in the two input runs. Without this, the last passes (a few very long
   runs) would be done by only a few threads.
3. **4-byte integers on the GPU**, keeping GPU memory and copy time to the minimum.
4. **One scratch buffer for the CPU merge sort**, allocated once.
5. **Messages longer than 2^31 elements** are sent in pieces, so the array size is not limited by MPI's 32-bit counts.

## How to run

```
sbatch scripts/run.sh                      # edit the partition and module lines for your cluster
N=4000000000 GPUCAP=1000000000 sbatch scripts/run.sh    # other sizes
# or by hand, on a node with a GPU:
make                                       # builds hybrid_sort (needs mpic++ and nvcc)
mpirun -np 32 ./hybrid_sort cpu 2000000000
mpirun -np 32 ./hybrid_sort hybrid 2000000000 500000000 4
make cpu_sort                              # CPU-only build, no CUDA needed
```

Timings are appended to `output/times.csv`.

## Results

One node with 32 CPU cores and one Tesla V100 (32 GB), 32 MPI processes, full log in
`output/results_V100.log`. **Data size: 2,000,000,000 integers (8 GB).** The GPU was allowed
500,000,000 integers, so the array is 4 times the GPU allowance. This allowance is a parameter set
below the real memory of the card so that the test fits in a short job: with the full 32 GB the
GPU sort could hold about 3.6 billion integers, and an array 4 times that would need about 58 GB plus the same again for merging.

| Run | per CPU process | on GPU | CPU local sort (s) | GPU sort (s) | merge (s) | total (s) |
|---|---|---|---|---|---|---|
| CPU only, 32 processes | 62.5 M | 0 | 7.12 | | 22.1 | 29.2 |
| hybrid, GPU share 4x | 57.1 M | 229 M | 6.54 | 5.60 | 24.7 | 31.2 |
| hybrid, GPU share 8x | 51.3 M | 410 M | 5.89 | 10.65 | 26.3 | 37.0 |
| hybrid, GPU share 16x (capped at 500 M) | 48.4 M | 500 M | 5.57 | 12.94 | 26.9 | 39.8 |

All runs are correct (in order, same elements as the input). Observations:

- **The sorting phase is faster with the GPU when the split is balanced:** 6.54 s against 7.12 s (8 % faster)
  at a share of 4. The GPU sorts about 41 million integers per second, one CPU core about 8.8 million,
  so the GPU is worth about 4.6 cores. Giving it more than that (shares 8 and 16) makes the GPU the slowest sorter.
- **The total time is not faster with the GPU** (31.2 s against 29.2 s). The merge takes three quarters of
  the time, and it gets slower in hybrid mode because the large GPU part makes one side of the merge tree heavier than the other.
- **The parallel merge** handles 2 billion integers in 22 s, about 11 ns per integer.

A further gain would need a different final step, for example splitting by value ranges (sample sort)
so that no single process has to merge the whole array.

## CUDA profile

`scripts/profile.sh` profiles a smaller hybrid run with Nsight Systems (8 processes, 400 million
integers, 100 million on the GPU; A100 node). Summary in `output/profile_hybrid_summary.txt`.

| Part of the GPU sort | Time (ms) |
|---|---|
| 27 merge passes (`gpu_mergesort` kernel), 5.0 to 9.4 ms each | 220 |
| copy to GPU (400 MB) | 49 |
| copy back (400 MB) | 43 |
| GPU sort total, as measured by the program | 486 |

The kernel takes about 45 % of the GPU process's time and the two copies about 19 %; most of the rest
is CUDA start-up (the first CUDA call, `cudaMemGetInfo`, takes 121 ms). In this run the GPU process
finishes in 0.49 s while each CPU process needs 5.3 s for 42.9 million integers, so on the A100 the GPU
could take a much larger share than the 100 million allowed here.
