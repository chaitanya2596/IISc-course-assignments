#include <iostream>
#include <string>
#include <vector>
#include <mpi.h>

#include "common.h"

using namespace std;

// Cannon's algorithm on a q x q process grid.
// Usage: mpirun -np <p> ./cannon <N> <A.bin> <B.bin> <C.bin>
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    if (argc < 5)
    {
        cerr << "usage: cannon <N> <A.bin> <B.bin> <C.bin>" << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int matSize = stoi(argv[1]);
    string fna = argv[2], fnb = argv[3], fnc = argv[4];

    Grid g;
    setupGrid(g, matSize);
    MPI_Comm comm_cart = g.cart;
    int *coords = g.coords;

    size_t local_buff_size = (size_t)g.nloc * g.nloc;
    vector<int> localDataA(local_buff_size), localDataB(local_buff_size), localDataC(local_buff_size, 0);

    // read from the files, each process only its own block
    MPI_Barrier(comm_cart);
    double start_time_read = MPI_Wtime();
    readBlock(g, fna, localDataA.data());
    readBlock(g, fnb, localDataB.data());
    double elapsed_time_read = MPI_Wtime() - start_time_read;

    MPI_Barrier(comm_cart);
    double start_time_compute = MPI_Wtime();

    // Initial realignment of the data
    int up_rank, down_rank, left_rank, right_rank;
    int rank_source_x, rank_dest_x, rank_source_y, rank_dest_y;
    // row i of A is shifted left by i blocks
    MPI_Cart_shift(comm_cart, 1, -coords[0], &rank_source_x, &rank_dest_x);
    MPI_Sendrecv_replace(localDataA.data(), local_buff_size, MPI_INT, rank_dest_x, 1, rank_source_x, 1, comm_cart, MPI_STATUS_IGNORE);
    // column j of B is shifted up by j blocks
    MPI_Cart_shift(comm_cart, 0, -coords[1], &rank_source_y, &rank_dest_y);
    MPI_Sendrecv_replace(localDataB.data(), local_buff_size, MPI_INT, rank_dest_y, 1, rank_source_y, 1, comm_cart, MPI_STATUS_IGNORE);

    MPI_Cart_shift(comm_cart, 1, -1, &right_rank, &left_rank);
    MPI_Cart_shift(comm_cart, 0, -1, &down_rank, &up_rank);

    for (int car = 0; car < g.q; car++)
    {
        // C block accumulates the product of the A and B blocks that meet here in this round
        matrix_multiply_add(localDataA.data(), localDataB.data(), localDataC.data(), g.nloc);

        if (car < g.q - 1)
        {
            // A moves one block to the left, B one block up
            MPI_Sendrecv_replace(localDataA.data(), local_buff_size, MPI_INT, left_rank, 1, right_rank, 1, comm_cart, MPI_STATUS_IGNORE);
            MPI_Sendrecv_replace(localDataB.data(), local_buff_size, MPI_INT, up_rank, 1, down_rank, 1, comm_cart, MPI_STATUS_IGNORE);
        }
    }

    double elapsed_time_compute = MPI_Wtime() - start_time_compute;

    MPI_Barrier(comm_cart);
    double start_time_write = MPI_Wtime();
    writeBlock(g, fnc, localDataC.data());
    double elapsed_time_write = MPI_Wtime() - start_time_write;

    reportTimes(g, "cannon", matSize, elapsed_time_read, elapsed_time_compute, elapsed_time_write);

    MPI_Finalize();
    return 0;
}
