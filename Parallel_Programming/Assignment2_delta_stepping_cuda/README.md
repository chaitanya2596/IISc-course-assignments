# Assignment 2: single-source shortest paths with delta-stepping on a GPU (CUDA)

## Problem

Question sheet: [question.pdf](question.pdf)

Implement parallel delta-stepping algorithm for finding Single Source Shortest Path, dealt in the
class, on GPU with CUDA. Implement GPU optimizations that are applicable, e.g., coalesced memory
access. Check the correctness of the results with a sequential CPU implementation.

Show results for the given graph (1,957,027 vertices, 2,760,388 undirected weighted edges):
https://drive.google.com/file/d/17mzIlsWRUclzwOzQlwgP9CLtz3olCfsa/view?usp=sharing

Perform experiments with source vertices 0, 500, 1000, 10000, 50000 and 100000. Show comparisons
of execution times with the CUDA version vs the sequential version. Also, for CUDA execution, run with
CUDA profiler, show and analyze the profiling outputs.

## What is implemented

Delta-stepping keeps every vertex in a "bucket" according to its current distance (bucket number =
distance / delta). Buckets are processed in increasing order. Edges with weight up to delta are
"light", the others "heavy". For the current bucket the light edges are relaxed again and again
until the bucket stays empty; then the heavy edges of every vertex that left the bucket are relaxed once.

| File | Content |
|---|---|
| `src/readMTXFile.cpp` | reads the MatrixMarket graph into an adjacency list |
| `src/adjListToCSR.cpp` | converts it to compressed row (CSR) arrays, light edges of each vertex first |
| `src/seqDeltaStepping.cpp` | sequential delta-stepping, and Dijkstra as an independent reference |
| `src/cudaKernel.cu` | the four GPU kernels: initialise, relax light edges, relax heavy edges, find next bucket |
| `src/main.cu` | runs sequential, Dijkstra and GPU for every source vertex and compares the distances |
| `src/seq_main.cpp` | CPU-only program (no GPU needed) |

GPU side, one thread per vertex in every kernel launch:
- **Light-edge kernel:** a vertex that is in the current bucket takes itself out and relaxes its
  light edges with `atomicMin`; any vertex whose distance drops is put into the bucket of its new
  distance. The host repeats the launch until a launch finds the bucket empty.
- **Heavy-edge kernel:** one launch per bucket for the vertices settled in it.
- **Next-bucket kernel:** one launch finds the smallest non-empty bucket, so empty buckets cost nothing.

## Optimizations

1. **Coalesced memory access:** distance, bucket and "settled" flag are three separate int arrays
   instead of an array of pairs, so neighbouring threads read neighbouring memory.
2. **Light and heavy edges split in the CSR arrays:** each kernel walks only its own part of a
   vertex's edge list, with no weight test per edge.
3. **Only 4 bytes cross between GPU and CPU per launch** (the "bucket still has work" flag); the
   distance array stays on the GPU until the end.
4. **Empty buckets are skipped** with the next-bucket kernel.
5. **Bucket width (delta) as a parameter.** This graph has few edges per vertex (2.8 on average) and
   long shortest paths (largest distance about 50,000), so a small delta means hundreds of buckets
   with very little work each, which suits a CPU and not a GPU. A larger delta gives the GPU more vertices per launch.
6. **Ordering of the atomic updates:** a vertex is put into a bucket only after its new distance is
   visible to all threads (memory fence between the two atomics), and shared values are read through a
   volatile pointer, so no thread works with an out-of-date distance.

## How to run

```
# put the graph at input/test_graph.mtx (link above), then on a cluster with Slurm and CUDA:
sbatch scripts/run_gpu.sh          # edit the partition and module lines for your cluster
# or by hand:
make                               # builds sssp (GPU) and sssp_seq (CPU only)
./sssp input/test_graph.mtx 0 1 500 1000 10000 50000 100000     # delta 0 = average edge weight + 1
./sssp input/test_graph.mtx 5000 1 500 1000 10000 50000 100000  # delta = 5000
./sssp_seq input/testCase_10.mtx 0 0                            # small test graph, CPU only
```

Arguments: `<graph file> <delta> <source> [<source> ...]`. Timings go to `output/timings_delta<d>.csv`.
`tools/testCaseGenerator.cpp` makes small random test graphs.

## Results

NVIDIA A100 (80 GB), full log in `output/results_A100.log`. Times are for the algorithm only (graph
already in memory; copying the graph to the GPU once takes about 0.2 s). Source 1 is used in place of
0, because vertex 0 has no edges in this graph (its vertices are numbered from 1).

**Correctness.** For every source and every delta, the GPU and sequential distances are identical to
Dijkstra's for all 1,957,028 vertices. A further 288 runs (8 repetitions x 6 values of delta x 6
sources, `scripts/stress.sh`, `output/stress_test_A100.log`) all match as well.

**Time against bucket width** (range over the six sources, in ms):

| delta | sequential | GPU | buckets | kernel launches |
|---|---|---|---|---|
| 101 (average weight + 1) | 140 to 144 | 125 to 150 | 478 to 588 | 4112 to 4989 |
| 200 | 124 to 128 | 83 to 100 | 243 to 297 | 2694 to 3274 |
| 1000 | 137 to 140 | 42 to 50 | 49 to 60 | 1320 to 1594 |
| 5000 | 219 to 232 | 32 to 37 | 10 to 12 | 899 to 1072 |
| 20000 | 529 to 674 | 31 to 35 | 3 | 765 to 875 |
| 100000 | 1245 to 1740 | 34 to 40 | 1 | 673 to 825 |

**Speedup, each side at its own best delta** (sequential at delta = 200, GPU at the better of 5000 and 20000):

| source | 1 | 500 | 1000 | 10000 | 50000 | 100000 |
|---|---|---|---|---|---|---|
| sequential (ms) | 125.4 | 125.3 | 128.3 | 125.1 | 124.0 | 123.6 |
| GPU (ms) | 33.7 | 31.2 | 31.9 | 32.8 | 30.6 | 34.9 |
| speedup | 3.7 | 4.0 | 4.0 | 3.8 | 4.1 | 3.5 |

At the default delta of 101 the two are about equal (speedup 0.95 to 1.15), because each launch then
touches only a few vertices; a larger delta gives each launch more work. At equal large delta the ratio
is 20 to 40, but there the sequential code runs far from its best delta.

## CUDA profile

`scripts/profile.sh` profiles source 1 with Nsight Systems at delta = 101 and delta = 5000
(`output/profile_delta*_summary.txt`).

| | delta = 101 | delta = 5000 |
|---|---|---|
| light-edge kernel | 3192 launches, 11.2 us each, 35.9 ms | 878 launches, 18.4 us each, 16.1 ms |
| next-bucket kernel | 483 launches, 45.0 us each, 21.8 ms | 10 launches, 0.5 ms |
| heavy-edge kernel | 483 launches, 12.5 us each, 6.0 ms | 10 launches, 0.1 ms |
| all kernels | 63.7 ms | 16.7 ms |

- The kernels are short (10 to 50 microseconds), so launching them and copying the 4-byte flag back after
  each light-edge launch costs about as much as the kernels themselves: at delta = 5000 the kernels take
  16.7 ms of the 33 ms total.
- At delta = 101 the next-bucket kernel, which reads the bucket of every vertex, takes a third of the GPU
  time because it runs 483 times. A larger delta removes most of these passes.
- The light-edge kernel does almost all the useful work. With a larger delta each launch has more vertices
  to process (18.4 us against 11.2 us per launch), but there are 3.6 times fewer launches.
