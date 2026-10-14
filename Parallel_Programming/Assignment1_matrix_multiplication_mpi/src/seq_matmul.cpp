#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>

#include "common.h"

using namespace std;
using namespace std::chrono;

// Sequential matrix multiplication with the same multiply routine as the parallel programs.
// Usage: ./seq_matmul <N> <A.bin> <B.bin> <C.bin>
static double secs(high_resolution_clock::time_point a, high_resolution_clock::time_point b)
{
    return duration_cast<microseconds>(b - a).count() / 1e6;
}

int main(int argc, char **argv)
{
    if (argc < 5)
    {
        cerr << "usage: seq_matmul <N> <A.bin> <B.bin> <C.bin>" << endl;
        return 1;
    }
    int n = stoi(argv[1]);
    size_t total = (size_t)n * n;
    vector<int> A(total), B(total), C(total, 0);

    auto t0 = high_resolution_clock::now();
    ifstream fa(argv[2], ios::binary), fb(argv[3], ios::binary);
    fa.read((char *)A.data(), total * sizeof(int));
    fb.read((char *)B.data(), total * sizeof(int));
    if (!fa || !fb)
    {
        cerr << "could not read the input matrices" << endl;
        return 1;
    }
    auto t1 = high_resolution_clock::now();

    matrix_multiply_add(A.data(), B.data(), C.data(), n);
    auto t2 = high_resolution_clock::now();

    ofstream fc(argv[4], ios::binary);
    fc.write((const char *)C.data(), total * sizeof(int));
    fc.close();
    auto t3 = high_resolution_clock::now();

    cout << "sequential  N = " << n << "  read " << secs(t0, t1) << " s  compute " << secs(t1, t2) << " s  write "
         << secs(t2, t3) << " s  total " << secs(t0, t3) << " s" << endl;
    ofstream outputTime("output/times.csv", ios::app);
    outputTime << "sequential," << n << ",1," << secs(t0, t1) << "," << secs(t1, t2) << "," << secs(t2, t3) << endl;
    return 0;
}
