#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdlib>

using namespace std;

// writes n random integers, comma separated, to input/input.txt
int main(int argc, char **argv)
{
    int n = (argc > 1) ? atoi(argv[1]) : 100000;
    ofstream input_file;
    input_file.open("input/input.txt");

    srand(12345);
    for (int i = 0; i < n; i++)
    {
        if (i != n - 1)
        {
            input_file << rand() % 1000000 + 1 << ", ";
        }
        else
        {
            input_file << rand() % 1000000 + 1;
        }
    }
    return 0;
}
