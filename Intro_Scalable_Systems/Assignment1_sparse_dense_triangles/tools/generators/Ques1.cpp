// generate a random dense matrix file of m*n order

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <time.h>

//
// Input to be given is only n i.e. initial order of matrix later it will be increased by 10 times
// ./a.out 1
//  we get 1, 10, 100, 1000, 10000

using namespace std;

int main(int argc, char *argv[])
{
    // check the number of arguments
    // if (argc != 2)
    // {
    //     cout << "Usage: " << argv[0] << " <m> <n> <filename>" << endl;
    //     return 1;
    // }

    // get the order of Sqaure Matrix
    // int n = atoi(argv[1]);
    int n = 0;

    int arr[4] = {1, 1, 10, 9};
    int nValues[4] = {5,10, 20, 400};
    int increments[4] = {0,0, 20, 200};

    for (size_t ii = 0; ii < 4; ii++)
    {
        n = nValues[ii];

        for (size_t m = 0; m < arr[ii]; m++)
        {
            // open the file
            string filename = "testSample_p1/in_" + to_string(n) + ".txt";
            ofstream file(filename);

            // check if the file is open
            if (!file.is_open())
            {
                cout << "Could not open the file" << endl;
                return 1;
            }

            // write the number of rows and columns
            file << n << endl;

            int a = 0;
            // generate the random matrix
            srand(time(NULL));
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n - 1; j++)
                {
                    a = rand() % 1000;
                    if (a > 600)
                    {
                        file << a % 100 << " ";
                    }
                    else
                    {
                        file << 0 << " ";
                    }
                }
                file << rand() % 100 << endl;
            }

            // close the file
            file.close();

            n += increments[ii];
        
        }
    }

    return 0;
}
