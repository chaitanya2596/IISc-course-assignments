#include <iostream>
#include <fstream>
#include <cmath>
#include "common.h"

using namespace std;

void setupGrid(Grid &g, int matSize)
{
    MPI_Comm_rank(MPI_COMM_WORLD, &g.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &g.size);

    int q = (int)lround(sqrt((double)g.size));
    if (q * q != g.size || matSize % q != 0)
    {
        if (g.rank == 0)
            cerr << "the number of processes must be a perfect square whose root divides the matrix size" << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    g.q = q;
    g.nloc = matSize / q;

    // cartesian communicator, periodic in both directions (needed for the circular shifts)
    int dims[2] = {q, q};
    int periods[2] = {1, 1};
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &g.cart);
    MPI_Comm_rank(g.cart, &g.rank);
    MPI_Cart_coords(g.cart, g.rank, 2, g.coords);

    int array_of_sizes[2] = {matSize, matSize};
    int localSubarraysizes[2] = {g.nloc, g.nloc};
    int array_of_starts[2] = {g.coords[0] * g.nloc, g.coords[1] * g.nloc};
    MPI_Type_create_subarray(2, array_of_sizes, localSubarraysizes, array_of_starts, MPI_ORDER_C, MPI_INT, &g.blockType);
    MPI_Type_commit(&g.blockType);
}

void readBlock(Grid &g, const string &fileName, int *localData)
{
    MPI_File file;
    int err = MPI_File_open(g.cart, fileName.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    if (err != MPI_SUCCESS)
    {
        if (g.rank == 0)
            cerr << "cannot open " << fileName << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    MPI_File_set_view(file, 0, MPI_INT, g.blockType, "native", MPI_INFO_NULL);
    MPI_File_read_all(file, localData, g.nloc * g.nloc, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);
}

void writeBlock(Grid &g, const string &fileName, int *localData)
{
    MPI_File file;
    MPI_File_open(g.cart, fileName.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file);
    MPI_File_set_view(file, 0, MPI_INT, g.blockType, "native", MPI_INFO_NULL);
    MPI_File_write_all(file, localData, g.nloc * g.nloc, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&file);
}

void reportTimes(Grid &g, const string &algo, int matSize, double tRead, double tCompute, double tWrite)
{
    double in[3] = {tRead, tCompute, tWrite}, out[3];
    MPI_Reduce(in, out, 3, MPI_DOUBLE, MPI_MAX, 0, g.cart);
    if (g.rank == 0)
    {
        cout << algo << "  N = " << matSize << "  processes = " << g.size << "  read " << out[0] << " s  compute " << out[1]
             << " s  write " << out[2] << " s  total " << out[0] + out[1] + out[2] << " s" << endl;
        ofstream outputTime("output/times.csv", ios::app);
        outputTime << algo << "," << matSize << "," << g.size << "," << out[0] << "," << out[1] << "," << out[2] << endl;
        outputTime.close();
    }
}
