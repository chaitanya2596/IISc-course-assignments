# Introduction to Scalable Systems (DS 221), Assignment 1: sparse and dense matrices, graph patterns

## Problem

The assignment comes with a test program (`src/main.cpp`) that reads the inputs, calls one function
per question, times it and writes the outputs. The solutions implement these functions:

| Question | Function | Task |
|---|---|---|
| Q1 | `denseToSparse` | convert an N x N dense matrix to compressed sparse row (CSR) form: values, column indices, row pointers |
| Q2a | `denseMatrixMultiplication` | multiply two N x N dense matrices |
| Q2b | `blockMatrixMultiplication` | the same product computed block by block, with block size B |
| Q2c | `sparseMatrixMultiplication` | multiply two N x N matrices given in CSR form; result in CSR form |
| Q3 | `findTrianglesAsMap`, `findTrianglesAsVec` | in a weighted directed graph (adjacency matrix), find every directed triangle i -> j -> k -> i and count how often each pattern of weights (w(i,j), w(j,k), w(k,i)) occurs; once with a `std::map`, once with a `std::vector` |

For each question the run times are recorded for growing N.

## What is implemented

All solutions are in `src/user_code.h`.

- **Q1** counts the non-zeros, sizes the three arrays once, and fills them row by row.
- **Q2a** is the triple loop over i, j, k.
- **Q2b** walks over B x B blocks (ii, jj, kk) and multiplies block pairs; blocks at the edge are cut at N, so B need not divide N.
- **Q2c** builds each result row in a dense work row: for every non-zero A(i,k) it adds A(i,k) times row k of B, then the non-zeros of the work row are stored in CSR form.
- **Q3** loops over i, j with an edge i -> j and over k, and keeps the triangles with edges j -> k and k -> i. The map version counts patterns with `map[pattern]++`. The vector version looks up each pattern with `std::find` and appends it if it is new. Every triangle is counted once per starting vertex, so its three rotations appear as three patterns.

## How to run

```
g++ -std=c++11 -O2 -o ds221_driver src/main.cpp
g++ -std=c++11 -O2 -o gen_inputs src/gen_inputs.cpp
sbatch scripts/run.sh            # or: bash scripts/run.sh ; SIZES="5 10 100" BLOCK=32 to choose sizes
```

`scripts/run.sh` generates inputs for each size (`src/gen_inputs.cpp`, fixed seed), runs the course
program, checks every output with `scripts/check.py`, and collects the times in `output/times.csv`.
The program can also be run on the course samples in `input/sample_p*`:

```
./ds221_driver input/sample_p1/in1.txt o1.txt input/sample_p2/ina1.txt o2a.txt input/sample_p2/inb1.txt o2b.txt \
               input/sample_p2/inc1.txt o2c.txt input/sample_p3/in1.txt o3.txt times.csv
```

`tools/generators/` holds per-question input generators.

## How the results were checked

`scripts/check.py` recomputes every answer with numpy, independently of the C++ code: CSR conversion,
the matrix products `A @ B`, the CSR product, and the triangle patterns (counted with array operations
over all j, k for each i). For Q3 both the map result and the vector result are compared.

All five problems match the reference on the samples in `input/` and on generated inputs for
N = 5, 10, 20, 50, 100, 200, 400, 800 (and N = 37 with block size 4, which does not divide N).

## Results

**Time per question in milliseconds** (CPU time measured by the course program, one core, compiled with -O2, block size 32; `output/times.csv`, full log `output/results.log`):

| N | Q1 dense to CSR | Q2a dense product | Q2b block product | Q2c sparse product | Q3 map | Q3 vector |
|---|---|---|---|---|---|---|
| 5 | 0.003 | 0.001 | 0.001 | 0.002 | 0.004 | 0.003 |
| 10 | 0.002 | 0.001 | 0.002 | 0.003 | 0.024 | 0.024 |
| 20 | 0.005 | 0.006 | 0.007 | 0.007 | 0.149 | 0.452 |
| 50 | 0.017 | 0.091 | 0.095 | 0.038 | 1.91 | 21 |
| 100 | 0.062 | 0.795 | 0.747 | 0.218 | 12.8 | 140 |
| 200 | 0.248 | 6.21 | 5.97 | 1.4 | 104 | 1.18e+03 |
| 400 | 1.02 | 59.9 | 47.8 | 9.41 | 800 | 9.17e+03 |
| 800 | 3.83 | 520 | 375 | 70.6 | 6.28e+03 | 8.18e+04 |

Observations (N = 800 unless stated):

- **Growth with N:** going from 400 to 800, Q1 takes 3.8 times longer (N^2 work, expected 4), the dense products 8.7 and 7.8 times (N^3, expected 8), Q3 about 7.8 (map) and 8.9 (vector) times (N^3 triangles).
- **Blocking helps once the matrices no longer fit in cache:** the block product is 1.39 times faster than the plain triple loop at N = 800 (375 ms against 520 ms), and about the same speed up to N = 200.
- **The sparse product is 7.4 times faster than the dense one** (71 ms against 520 ms) with about 40 % non-zeros, because it only multiplies non-zero pairs: the work falls with the square of the density (about 0.16 of the dense count here).
- **Map against vector for Q3:** the map is 13 times faster (6.3 s against 81.8 s for 46.5 million triangles). With weights 0 to 9 there are at most 729 different patterns: the map finds one in about log2(729) = 10 comparisons, the vector searches its list from the start each time (on average about half of it).
