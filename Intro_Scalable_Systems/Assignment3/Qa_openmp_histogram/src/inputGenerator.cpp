#include <iostream>
#include <cmath>
#include <fstream>
#include <cstdlib>

using namespace std;

// writes n random integers between 1 and distinct_values, comma separated, to input/input.txt
int main(int argc, char **argv)
{
    int n = (argc > 1) ? atoi(argv[1]) : 200000;
    int distinct_values = (argc > 2) ? atoi(argv[2]) : 50;
    ofstream input_file;
    input_file.open("input/input.txt");

    srand(12345);
    for (int i = 0; i < n; i++)
    {
        if (i != n - 1)
        {
            input_file << rand() % distinct_values + 1 << ", ";
        }
        else
        {
            input_file << rand() % distinct_values + 1;
        }
    }
    return 0;
}
