#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <mpi.h>

#include "distributionCalc.h"
#include "mergeAlgo.h"
#ifdef USE_GPU
#include "mergeGpu.h"
#endif

using namespace std;

// Integer sort of a large array with MPI processes on the CPU cores and, in hybrid mode, one
// process that sorts its part on the GPU at the same time.
//
// Usage: mpirun -np <P> ./hybrid_sort <mode: cpu | hybrid> <n> [gpuCapacity] [gpuWeight]
//   n            number of integers to sort
//   gpuCapacity  largest number of integers the GPU process may take (default: what fits in the
//                free GPU memory; the GPU sort needs two buffers of that size)
//   gpuWeight    share of the GPU process relative to one CPU process (default 4)
//
// Steps: 1. every process creates its own part of the input (not timed)
//        2. local sort, CPU processes with merge sort, the GPU process on the GPU
//        3. parallel merge: the processes merge in pairs, the survivors merge in pairs again, ...
//           until the whole sorted array is in process 0 (log2(P) rounds, the merges of one round
//           run at the same time on different processes)
//        4. process 0 checks the result

// pseudo-random value of the element with global index i (the input does not depend on P)
static inline int valueOf(unsigned long i)
{
    unsigned long z = i + 0x9E3779B97F4A7C15UL;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9UL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBUL;
    z = z ^ (z >> 31);
    return (int)(z & 0x7FFFFFFF);
}

// MPI counts are 32-bit, so long arrays are sent in pieces
static const long CHUNK = 1L << 28;
static void sendLong(const int *buf, long count, int dest)
{
    MPI_Send(&count, 1, MPI_LONG, dest, 0, MPI_COMM_WORLD);
    for (long off = 0; off < count; off += CHUNK)
    {
        MPI_Send(buf + off, (int)min(CHUNK, count - off), MPI_INT, dest, 1, MPI_COMM_WORLD);
    }
}
static void recvLong(vector<int> &buf, int source)
{
    long count;
    MPI_Recv(&count, 1, MPI_LONG, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    buf.resize(count);
    for (long off = 0; off < count; off += CHUNK)
    {
        MPI_Recv(buf.data() + off, (int)min(CHUNK, count - off), MPI_INT, source, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3)
    {
        if (rank == 0)
            cerr << "usage: hybrid_sort <cpu|hybrid> <n> [gpuCapacity] [gpuWeight]" << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    string mode = argv[1];
    long n = atol(argv[2]);
    long gpuCapacity = (argc > 3) ? atol(argv[3]) : 0;
    double gpuWeight = (argc > 4) ? atof(argv[4]) : 4.0;
    int gpuRanks = (mode == "hybrid") ? 1 : 0;
    bool iAmGpu = (gpuRanks == 1 && rank == size - 1);

#ifndef USE_GPU
    if (gpuRanks == 1)
    {
        if (rank == 0)
            cerr << "this executable was built without GPU support (use 'make hybrid_sort')" << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
#else
    if (gpuRanks == 1)
    {
        // the GPU process reports how much the GPU can hold: two int buffers, 10 % kept free
        long cap = 0;
        if (iAmGpu)
        {
            long freeB, totalB;
            gpuMemory(&freeB, &totalB);
            cap = (long)(0.9 * freeB) / (2 * sizeof(int));
        }
        long capAll;
        MPI_Allreduce(&cap, &capAll, 1, MPI_LONG, MPI_MAX, MPI_COMM_WORLD);
        if (gpuCapacity <= 0 || gpuCapacity > capAll)
            gpuCapacity = capAll;
    }
#endif

    vector<long> distributionarray, displacementarray;
    distriCalc(n, size, gpuRanks, gpuWeight, gpuCapacity, distributionarray, displacementarray);
    long myCount = distributionarray[rank];

    if (rank == 0)
    {
        cout << "mode = " << mode << ", processes = " << size << ", elements = " << n << " (" << n * 4.0 / 1e9 << " GB)" << endl;
        cout << "elements per CPU process = " << distributionarray[0];
        if (gpuRanks == 1)
            cout << ", elements on the GPU process = " << distributionarray[size - 1] << " (GPU capacity used for the split = "
                 << gpuCapacity << ", array / capacity = " << (double)n / gpuCapacity << ")";
        cout << endl;
    }

    // ---------------- 1. input: each process fills its own part ----------------
    vector<int> myData(myCount);
    long mySum = 0;
    int myXor = 0;
    for (long i = 0; i < myCount; i++)
    {
        int v = valueOf(displacementarray[rank] + i);
        myData[i] = v;
        mySum += v;
        myXor ^= v;
    }
    long inputSum;
    int inputXor;
    MPI_Reduce(&mySum, &inputSum, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&myXor, &inputXor, 1, MPI_INT, MPI_BXOR, 0, MPI_COMM_WORLD);

    // ---------------- 2. local sort ----------------
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    double gpuSeconds = 0;
    if (iAmGpu)
    {
#ifdef USE_GPU
        gpuSeconds = gpuMergeSort(myData.data(), myCount);
#endif
    }
    else
    {
        mergeSort(myData.data(), myCount);
    }
    double elapsed_time_local_compute = MPI_Wtime() - start_time;

    double max_local, max_cpu_local, cpuLocal = iAmGpu ? 0 : elapsed_time_local_compute, gpuLocal;
    MPI_Reduce(&elapsed_time_local_compute, &max_local, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&cpuLocal, &max_cpu_local, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&gpuSeconds, &gpuLocal, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // ---------------- 3. parallel merge (tree) ----------------
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time_merge = MPI_Wtime();

    vector<int> recvbuf, merged;
    for (int step = 1; step < size; step *= 2)
    {
        if (rank % (2 * step) == 0)
        {
            int partner = rank + step;
            if (partner < size)
            {
                recvLong(recvbuf, partner);
                merged.resize(myData.size() + recvbuf.size());
                mergeTwo(myData.data(), myData.size(), recvbuf.data(), recvbuf.size(), merged.data());
                myData.swap(merged);
                vector<int>().swap(recvbuf);
                vector<int>().swap(merged);
            }
        }
        else
        {
            sendLong(myData.data(), myData.size(), rank - step);
            vector<int>().swap(myData);
            break;
        }
    }
    double end_time = MPI_Wtime();

    // ---------------- 4. check and report (process 0 holds the sorted array) ----------------
    if (rank == 0)
    {
        bool sorted = ((long)myData.size() == n);
        long outSum = 0;
        int outXor = 0;
        for (long i = 0; i < (long)myData.size(); i++)
        {
            if (i > 0 && myData[i - 1] > myData[i])
                sorted = false;
            outSum += myData[i];
            outXor ^= myData[i];
        }
        bool sameElements = (outSum == inputSum && outXor == inputXor);
        bool ok = sorted && sameElements;

        double mergeTime = end_time - start_time_merge;
        double total = end_time - start_time;
        cout << "local sort (slowest process): " << max_local << " s   [slowest CPU process " << max_cpu_local << " s";
        if (gpuRanks == 1)
            cout << ", GPU process " << gpuLocal << " s";
        cout << "]" << endl;
        cout << "parallel merge              : " << mergeTime << " s" << endl;
        cout << "total sort time             : " << total << " s" << endl;
        cout << "result: " << (sorted ? "in order" : "NOT in order") << ", " << (sameElements ? "same elements as the input (sum and xor match)" : "elements DIFFER from the input")
             << "  -->  " << (ok ? "CORRECT" : "WRONG") << endl << endl;

        ofstream csv("output/times.csv", ios::app);
        csv << mode << "," << size << "," << n << "," << distributionarray[0] << "," << (gpuRanks ? distributionarray[size - 1] : 0) << ","
            << max_cpu_local << "," << gpuLocal << "," << mergeTime << "," << total << "," << (ok ? "yes" : "NO") << endl;
    }

    MPI_Finalize();
    return 0;
}
