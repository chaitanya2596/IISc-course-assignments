# DS 221 Assignment 3: OpenMP histogram and MPI merge sort

Question sheet: [question.pdf](question.pdf)

Two questions, each in its own folder with `src/`, `input/`, `output/`, `scripts/run.sh` and a `Makefile`.

```
cd Qa_openmp_histogram        # or Qb_mpi_merge_sort
sbatch scripts/run.sh         # Slurm; edit the partition and module lines for your cluster
bash scripts/run.sh           # or run directly on a machine with enough cores
```

`run.sh` builds the code, generates the input file if it is missing, runs every case and writes
the timings to `output/`. Qa needs `g++` with OpenMP, Qb needs an MPI compiler (`mpic++`).

## Qa. Parallelization of histogram calculation using OpenMP

**Problem.** Generate an array of 2,00,000 integers between 1 and 50. For a histogram interval h the
histogram array has 50/h elements (interval 5: count of 1-5 in element 0, 6-10 in element 1, ...).
Write a sequential program, then parallelize it with OpenMP so that different threads process
different elements of the input; input and histogram arrays are shared. Check against the
sequential output. Time 2, 4, 8, 16, 32 threads for intervals 2, 5, 10 and 25, averaging 5 runs.

**What is implemented** (`src/histogram.cpp`). Three OpenMP versions next to the sequential one:
- `atomic`: shared histogram, every increment under `#pragma omp atomic`;
- `critical`: shared histogram, every increment inside `#pragma omp critical`;
- `local`: every thread counts into its own private array and adds it to the shared histogram once
  at the end, so the shared array is locked once per thread instead of once per element.

**Results** (one 32-core node, time of one histogram in microseconds, `output/results.log`). All parallel results match the sequential one.

| interval | sequential | local, 2 threads | 4 | 8 | 16 | 32 |
|---|---|---|---|---|---|---|
| 2 | 1165 | 577 | 308 | 162 | 73.7 | 62.2 |
| 5 | 1068 | 560 | 300 | 158 | 73.0 | 58.3 |
| 10 | 1015 | 547 | 302 | 155 | 70.3 | 56.9 |
| 25 | 992 | 534 | 288 | 156 | 68.6 | 57.1 |

| speedup of `local` | 2 threads | 4 | 8 | 16 | 32 |
|---|---|---|---|---|---|
| interval 2 | 2.0 | 3.8 | 7.2 | 15.8 | 18.7 |
| interval 5 | 1.9 | 3.6 | 6.8 | 14.6 | 18.3 |
| interval 10 | 1.9 | 3.4 | 6.6 | 14.4 | 17.9 |
| interval 25 | 1.9 | 3.4 | 6.4 | 14.5 | 17.4 |

The `atomic` version is 2 to 5 times slower than the sequential code at every thread count (for
example 2105 us on 2 threads and 5783 us on 32 threads for interval 2), and `critical` is slower
still: with only 2 to 25 bins all threads keep writing to the same few memory locations, so they
spend their time waiting for each other. The private-array version removes that completely.
Beyond 16 threads the gain flattens because one histogram is then only about 60 us of work.

**Optimizations**
1. Private per-thread counts merged once at the end, padded so two threads never share a cache line.
2. `schedule(static)`: each thread gets one contiguous block of the input.
3. The input is passed by reference, and the bin is computed as `(value - 1) / interval`.
4. Threads are pinned to cores (`OMP_PROC_BIND`, `OMP_PLACES`).
5. Timing: one histogram takes about 1 ms, the same order as starting the threads and as the timer
   resolution, so one run repeats the histogram 200 times and reports the time of one; 5 runs are averaged.

## Qb. Merge sort with MPI

**Problem.** Implement a parallel merge sort of a 1 lakh integer array using MPI. Process 0 reads
the elements from an input file and distributes them. Each process sorts locally. Then the processes
get together in groups of two and merge their sorted arrays; the representatives of these groups do
the same, and so on, until the entire sorted array is formed in process 0. Run with 8, 16, 32, 64
processes and calculate the speedups with respect to sequential quick sort. Also compare with a
version in which all processes send their sorted arrays to process 0, which does the merging.
Average 5 runs for each experiment.

**What is implemented** (`src/main.cpp`, `src/mergeSort.cpp`).
- Version A, tree: `MPI_Scatterv`, local merge sort, then log2(P) rounds in which the process with
  rank `r + step` sends its run to rank `r`, which merges the two.
- Version B, process 0: `MPI_Scatterv`, local merge sort, `MPI_Gatherv`, then process 0 merges
  neighbouring runs two at a time until one is left.
- Baseline: C library `qsort` on process 0. Both parallel results are compared element by element with it.

The timed section runs from the unsorted array in the memory of process 0 to the sorted array in
the memory of process 0 (distribution, sorting, all communication and merging). Reading the text file is not timed in any version.

**Results** (1,00,000 integers, average of 5 runs, times in ms, `output/results.log`). All results are correct.

| processes | quick sort | tree (A) | speedup A | process 0 (B) | speedup B |
|---|---|---|---|---|---|
| 1 | 12.54 | 7.82 | 1.60 | 7.80 | 1.61 |
| 8 | 12.72 | 1.83 | 6.93 | 2.50 | 5.10 |
| 16 | 12.54 | 1.52 | 8.23 | 2.59 | 4.84 |
| 32 | 12.52 | 1.47 | 8.50 | 3.02 | 4.14 |
| 64 (two nodes) | 12.57 | 5.44 | 2.31 | 5.93 | 2.12 |

- The tree version keeps improving up to 32 processes; version B gets slower from 8 processes on,
  because process 0 alone has to gather and merge more and more pieces.
- The 1-process row shows that this merge sort is itself 1.6 times faster than `qsort` (which calls
  a comparison function for every pair). Relative to the 1-process merge sort the tree version is 4.3, 5.1 and 5.3 times faster on 8, 16 and 32 processes.
- 64 processes do not fit on one 32-core node. Across two nodes the run takes 5.4 ms: the whole job
  is only about 1.5 ms of work, and messages between nodes (over TCP here) cost more than that. It is still 2.3 times faster than the sequential quick sort.

**Optimizations**
1. Local sort with a single scratch buffer allocated once.
2. Merges write straight into preallocated buffers.
3. The process-0 version merges runs two at a time (about log2(P) passes over the data).
4. Counts that do not divide evenly are spread over the processes.
5. The timed section excludes reading the text file, and one untimed warm-up run opens the MPI connections first.

Note: on the cluster used, the default Open MPI transport (UCX) failed for runs spanning two nodes,
so `scripts/run.sh` selects TCP between nodes. Clear `MPI_FLAGS` in the script if your cluster does not need this.
