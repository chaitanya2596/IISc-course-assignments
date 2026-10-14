#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <cstdlib>

using namespace std;

// Writes the five input files for one matrix size n, in the formats read by main.cpp:
//   <dir>/p1_<n>.txt   dense matrix (Q1)              : n, then n x n values
//   <dir>/p2a_<n>.txt  two dense matrices (Q2a)       : n, then A and B
//   <dir>/p2b_<n>.txt  two dense matrices + block (Q2b): n B, then A and B
//   <dir>/p2c_<n>.txt  two sparse matrices (Q2c)      : n, then values / column indices / row pointers of A, then of B
//   <dir>/p3_<n>.txt   weighted directed graph (Q3)   : n, then the adjacency matrix (0 = no edge, zero diagonal)
// Values are 0..9; the sparse matrices have about 60 % zeros. Fixed seed, so the files are reproducible.
// Usage: ./gen_inputs <n> <block size B> <dir>
int main(int argc, char **argv)
{
    if (argc < 4)
    {
        cerr << "usage: gen_inputs <n> <B> <dir>" << endl;
        return 1;
    }
    int n = atoi(argv[1]), B = atoi(argv[2]);
    string dir = argv[3], tag = to_string(n);
    mt19937 gen(12345 + n);
    uniform_int_distribution<int> digit(0, 9), percent(0, 99);

    auto dense = [&](ofstream &f, bool zeroDiagonal, int sparsePercent) {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                int v = digit(gen);
                if (zeroDiagonal && i == j)
                    v = 0;
                if (percent(gen) < sparsePercent)
                    v = 0;
                f << v << (j + 1 < n ? " " : "\n");
            }
        }
    };
    auto sparse = [&](ofstream &f) {
        vector<int> vals, cols, rows(1, 0);
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                int v = (percent(gen) < 60) ? 0 : 1 + digit(gen) % 9;
                if (v != 0)
                {
                    vals.push_back(v);
                    cols.push_back(j);
                }
            }
            rows.push_back(vals.size());
        }
        for (size_t k = 0; k < vals.size(); k++) f << vals[k] << " ";
        f << "\n";
        for (size_t k = 0; k < cols.size(); k++) f << cols[k] << " ";
        f << "\n";
        for (size_t k = 0; k < rows.size(); k++) f << rows[k] << " ";
        f << "\n";
    };

    ofstream p1(dir + "/p1_" + tag + ".txt");
    p1 << n << "\n";
    dense(p1, false, 30);
    ofstream p2a(dir + "/p2a_" + tag + ".txt");
    p2a << n << "\n";
    dense(p2a, false, 0);
    dense(p2a, false, 0);
    ofstream p2b(dir + "/p2b_" + tag + ".txt");
    p2b << n << " " << B << "\n";
    dense(p2b, false, 0);
    dense(p2b, false, 0);
    ofstream p2c(dir + "/p2c_" + tag + ".txt");
    p2c << n << "\n";
    sparse(p2c);
    sparse(p2c);
    ofstream p3(dir + "/p3_" + tag + ".txt");
    p3 << n << "\n";
    dense(p3, true, 50);
    return 0;
}
