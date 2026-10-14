#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <string>

using namespace std;

// generates the two n x n input matrices (integers 0..9) as raw binary files of 4-byte ints,
// row by row:  <dir>/mat_<n>_A.bin and <dir>/mat_<n>_B.bin
// Usage: ./Mat_generator <n> [dir = input]
int main(int argc, char **argv)
{
    int n = (argc > 1) ? stoi(argv[1]) : 16;
    string dir = (argc > 2) ? argv[2] : "input";
    int range = 10;
    vector<string> name = {"A", "B"};

    for (int k = 0; k < 2; k++)
    {
        mt19937 gen(1000 + k); // fixed seeds, so the same matrices are produced every time
        uniform_int_distribution<int> value(0, range - 1);
        ofstream fout(dir + "/mat_" + to_string(n) + "_" + name[k] + ".bin", ios::binary);
        vector<int> row(n);
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                row[j] = value(gen);
            }
            fout.write((const char *)row.data(), n * sizeof(int));
        }
        fout.close();
    }
    return 0;
}
