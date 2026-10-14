# Assignment 1: dense matrix-matrix multiplication with MPI

## Problem

Question sheet: [question.pdf](question.pdf)

There are many parallel algorithms for dense matrix-matrix multiplication. Explore and find a good
parallel algorithm, implement it by yourself using MPI. Also, implement Cannon's algorithm dealt in
the class. Perform experiments and compare the two algorithms for three matrix inputs
(6400 x 6400, 12800 x 12800, 25600 x 25600 for A, B and C) and 16 and 64 processor cores.

The two matrices A and B should be read from file and the C matrix should be stored in file using
MPI I/O interfaces. Also, compare with sequential matrix-matrix multiplication, plot the execution time and speedup
curves, and prepare a report containing the method, graphs and observations.

## What is implemented

Both parallel programs use a q x q grid of processes (q = 4 for 16 processes, q = 8 for 64). Each
process owns one (N/q) x (N/q) block of A, of B and of C, and never holds a full matrix.

| File | Content |
|---|---|
| `src/cannon.cpp` | **Cannon's algorithm.** Row i of the A blocks is first shifted left by i, column j of the B blocks up by j. Then q rounds: multiply the local blocks and add to C, shift A one block left and B one block up. |
| `src/summa.cpp` | **SUMMA** (the algorithm behind ScaLAPACK's matrix multiply). In round k the processes of grid column k broadcast their A block along their row, the processes of grid row k broadcast their B block along their column, and every process adds the product of the two received blocks to its C block. |
| `src/common.cpp`, `src/common.h` | process grid, MPI-IO, and the block multiply routine shared by all three programs |
| `src/seq_matmul.cpp` | sequential program using the same multiply routine |
| `src/Mat_generator.cpp` | writes the input matrices (integers 0 to 9, fixed seeds) as binary files |

**MPI-IO.** A, B and C are raw binary files of 4-byte integers. Each process reads and writes only
its own block through a subarray file view (`MPI_Type_create_subarray`, `MPI_File_set_view`,
`MPI_File_read_all`, `MPI_File_write_all`).

**Block multiply.** Loop order i-k-j, so B and C are walked along rows, with the k and j loops cut
into tiles of 256 so the part of B in use stays in cache. The sequential program uses exactly the
same routine, so the speedups compare like with like.

## How to run

```
sbatch scripts/run.sh                              # all sizes, 16 and 64 processes (about 2 h 10 min)
SIZES="6400" PROCS="16" sbatch scripts/run.sh      # a subset
# or by hand:
make
./Mat_generator 6400 input
./seq_matmul 6400 input/mat_6400_A.bin input/mat_6400_B.bin output/C_seq.bin
mpirun -np 16 ./cannon 6400 input/mat_6400_A.bin input/mat_6400_B.bin output/C_cannon.bin
mpirun -np 16 ./summa  6400 input/mat_6400_A.bin input/mat_6400_B.bin output/C_summa.bin
cmp output/C_seq.bin output/C_cannon.bin           # no output = identical
```

The number of processes must be a perfect square whose root divides N. The job script generates the
matrices, runs everything, compares every result file with the sequential one, deletes the large
files (2.6 GB each at N = 25600) and writes `output/times.csv`, `output/speedup.csv` and `output/correctness.txt`.

## Results

Two 32-core nodes; the sequential program and the 16-process runs use one node, the 64-process runs
both. Full log in `output/results.log`.

**Correctness.** All 12 parallel runs (2 algorithms x 3 sizes x 2 process counts) produce a C file
identical to the sequential one (`output/correctness.txt`); on small matrices the sequential result
was also checked against numpy.

**Multiplication time in seconds** (speedup over sequential in brackets):

| N | sequential | Cannon, 16 | SUMMA, 16 | Cannon, 64 | SUMMA, 64 |
|---|---|---|---|---|---|
| 6400 | 77.7 | 4.49 (17.3) | 4.50 (17.3) | 2.59 (30.1) | 5.49 (14.2) |
| 12800 | 637.9 | 35.6 (17.9) | 35.6 (17.9) | 14.6 (43.7) | 24.1 (26.5) |
| 25600 | 5479.1 | 297.8 (18.4) | 293.3 (18.7) | 93.5 (58.6) | 126.3 (43.4) |

**Total time including reading A, B and writing C** (speedup in brackets):

| N | sequential | Cannon, 16 | SUMMA, 16 | Cannon, 64 | SUMMA, 64 |
|---|---|---|---|---|---|
| 6400 | 79.6 | 10.0 (7.9) | 9.9 (8.0) | 8.7 (9.2) | 11.5 (7.0) |
| 12800 | 649.8 | 56.8 (11.4) | 56.2 (11.6) | 36.6 (17.8) | 46.0 (14.1) |
| 25600 | 5505.8 | 392.6 (14.0) | 388.1 (14.2) | 178.8 (30.8) | 213.0 (25.9) |

Observations:
- **16 processes:** both algorithms take the same time, with speedups slightly above 16 (17.3 to 18.7),
  as each process works on blocks of N/4 that fit in cache better than the full matrices.
- **64 processes:** Cannon is faster than SUMMA (58.6 against 43.4 at N = 25600). These runs span two
  nodes. Cannon sends each block only to a neighbour, SUMMA broadcasts each block to 7 other
  processes in every round, so SUMMA moves more data over the link between the nodes.
- **Larger matrices scale better:** on 64 processes Cannon reaches 30.1, 43.7 and 58.6 for the three
  sizes, because the multiplication grows as N^3 while the data sent grows as N^2.
- **File I/O limits the total:** at N = 25600 on 64 processes, reading and writing take 85 s of the
  179 s. The parallel read is slower than the sequential one (about 50 s against 1 s) because 64
  processes pull interleaved pieces of the same files from a shared file system.
