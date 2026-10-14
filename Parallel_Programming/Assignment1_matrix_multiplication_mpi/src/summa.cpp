#include <iostream>
#include <string>
#include <vector>
#include <mpi.h>

#include "common.h"

using namespace std;

// SUMMA (Scalable Universal Matrix Multiplication Algorithm, the method used by ScaLAPACK's
// PDGEMM) on a q x q process grid. In round k
//   - the processes of grid column k broadcast their A block along their grid row,
//   - the processes of grid row k broadcast their B block along their grid column,
//   - every process adds (received A block) * (received B block) to its C block.
// Each process stores only its own blocks of A, B, C plus two receive buffers, so no process ever
// holds a full matrix.
// Usage: mpirun -np <p> ./summa <N> <A.bin> <B.bin> <C.bin>
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    if (argc < 5)
    {
        cerr << "usage: summa <N> <A.bin> <B.bin> <C.bin>" << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int matSize = stoi(argv[1]);
    string fna = argv[2], fnb = argv[3], fnc = argv[4];

    Grid g;
    setupGrid(g, matSize);

    // communicators of the processes that share a grid row / a grid column
    MPI_Comm row_comm, col_comm;
    int keepRow[2] = {0, 1}; // keep the column index free --> all processes of one row
    int keepCol[2] = {1, 0}; // keep the row index free    --> all processes of one column
    MPI_Cart_sub(g.cart, keepRow, &row_comm);
    MPI_Cart_sub(g.cart, keepCol, &col_comm);

    size_t local_buff_size = (size_t)g.nloc * g.nloc;
    vector<int> localDataA(local_buff_size), localDataB(local_buff_size), localDataC(local_buff_size, 0);
    vector<int> bufA(local_buff_size), bufB(local_buff_size);

    MPI_Barrier(g.cart);
    double start_time_read = MPI_Wtime();
    readBlock(g, fna, localDataA.data());
    readBlock(g, fnb, localDataB.data());
    double elapsed_time_read = MPI_Wtime() - start_time_read;

    MPI_Barrier(g.cart);
    double start_time_compute = MPI_Wtime();

    for (int k = 0; k < g.q; k++)
    {
        // rank of the broadcasting process inside the row / column communicator
        int rootInRow, rootInCol;
        int ck[1] = {k};
        MPI_Cart_rank(row_comm, ck, &rootInRow);
        MPI_Cart_rank(col_comm, ck, &rootInCol);

        int *aBlock = bufA.data(), *bBlock = bufB.data();
        if (g.coords[1] == k)
            aBlock = localDataA.data(); // this process owns A(i,k) and sends it
        if (g.coords[0] == k)
            bBlock = localDataB.data(); // this process owns B(k,j) and sends it

        MPI_Bcast(aBlock, local_buff_size, MPI_INT, rootInRow, row_comm);
        MPI_Bcast(bBlock, local_buff_size, MPI_INT, rootInCol, col_comm);

        matrix_multiply_add(aBlock, bBlock, localDataC.data(), g.nloc);
    }

    double elapsed_time_compute = MPI_Wtime() - start_time_compute;

    MPI_Barrier(g.cart);
    double start_time_write = MPI_Wtime();
    writeBlock(g, fnc, localDataC.data());
    double elapsed_time_write = MPI_Wtime() - start_time_write;

    reportTimes(g, "summa", matSize, elapsed_time_read, elapsed_time_compute, elapsed_time_write);

    MPI_Finalize();
    return 0;
}
