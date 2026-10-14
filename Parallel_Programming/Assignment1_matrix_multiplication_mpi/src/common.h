#ifndef COMMON_H
#define COMMON_H
#include <mpi.h>
#include <string>
#include <vector>
#include <algorithm>

// C += A * B for square n x n blocks stored row by row.
// The same routine is used by the sequential program and inside both parallel algorithms, so the
// speedups compare like with like. Loop order i-k-j walks B and C along rows (unit stride), and
// the k and j loops are tiled so the part of B being used stays in cache for large n.
static inline void matrix_multiply_add(const int *matA, const int *matB, int *result, int n)
{
    const int tile = 256;
    for (int kk = 0; kk < n; kk += tile)
    {
        int kend = std::min(kk + tile, n);
        for (int jj = 0; jj < n; jj += tile)
        {
            int jend = std::min(jj + tile, n);
            for (int i = 0; i < n; ++i)
            {
                int *c = result + (size_t)i * n;
                for (int k = kk; k < kend; ++k)
                {
                    int a = matA[(size_t)i * n + k];
                    const int *b = matB + (size_t)k * n;
                    for (int j = jj; j < jend; ++j)
                    {
                        c[j] += a * b[j];
                    }
                }
            }
        }
    }
}

// 2D process grid shared by both algorithms: q x q processes, each owns a (N/q) x (N/q) block
struct Grid
{
    MPI_Comm cart;
    int q;         // processes per side
    int coords[2]; // (row, column) of this process
    int rank, size;
    int nloc;      // rows (= columns) of the local block
    MPI_Datatype blockType; // subarray type describing the local block inside the N x N file
};

// creates the grid; aborts if the process count is not a perfect square or does not divide N
void setupGrid(Grid &g, int matSize);

// MPI-IO: every process reads / writes only its own block through a subarray file view
void readBlock(Grid &g, const std::string &fileName, int *localData);
void writeBlock(Grid &g, const std::string &fileName, int *localData);

// collects the per-process maximum of the three phase times on process 0, prints and stores them
void reportTimes(Grid &g, const std::string &algo, int matSize, double tRead, double tCompute, double tWrite);

#endif
