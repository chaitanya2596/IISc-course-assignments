// generate a random dense matrix file of m*n order

#include <fstream>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <string>
#include <utility>
#include <map>
#include <unordered_map>
#include <iostream>
#include <algorithm>

using namespace std;

vector<vector<vector<int>>> twoRandomSparseMatrix(int n)
{

    // generate a random sparse matrix
    vector<vector<vector<int>>> twoSparseMatrix;
    srand(time(0));
    int k = 0;
    while (k < 2)
    {
        int a;
        vector<vector<int>> twoDMatrix;
        for (int x = 0; x < n; x++)
        {
            vector<int> oneDMatrix;
            for (int y = 0; y < n - 1; y++)
            {
                a = rand() % 100;
                if (a < 60)
                {
                    oneDMatrix.push_back(a);
                }
                else
                {
                    oneDMatrix.push_back(0);
                }
            }
            oneDMatrix.push_back(rand() % 10);
            twoDMatrix.push_back(oneDMatrix);
        }
        twoSparseMatrix.push_back(twoDMatrix);
        k = k + 1;
    }

    return twoSparseMatrix;
}

//  Function to convert a dense matrix to a sparse matrix representation
void denseToSparse(const std::vector<std::vector<int>> &denseMatrix,
                   std::vector<int> &values,
                   std::vector<int> &columnIndices,
                   std::vector<int> &rowPointers)
{
    // Write your code here
    int numRows = denseMatrix.size();
    int numCols = denseMatrix[0].size();

    // cout << "number of rows = " << denseMatrix.size() << endl;
    // cout << "number of columns = " << denseMatrix[0].size() << endl;

    // get the number of non-zero elements
    int numNonZero = 0;
    for (int z = 0; z < numRows; z++)
    {
        numNonZero += count_if(denseMatrix[z].begin(), denseMatrix[z].end(), [](int x)
                               { return x != 0; });
        // cout << "number of non-zero elements = " << numNonZero << endl;
    }

    if (numNonZero == 0)
    {
        // cout << "number of non-zero elements = " << numNonZero << endl;
        cout << "denseMatrix is empty" << endl;
        return;
    }
    // cout << "number of non-zero elements = " << numNonZero << endl;

    // resize the vectors
    // cout << "numRows : " << numRows << endl;
    rowPointers.resize(numRows + 1);
    columnIndices.resize(numNonZero);
    values.resize(numNonZero);

    // cout << "size of rowPointers = " << rowPointers.size() << endl;
    // cout << "size of columnIndices = " << columnIndices.size() << endl;
    // cout << "size of values = " << values.size() << endl;

    // cout << "row Ptr size = " << rowPointers.size() << endl;

    // fill the vectors
    int k = 0;
    for (int i = 0; i < numRows; i++)
    {
        rowPointers[i] = k;
        // cout << "rowPointers[" << i << "] = " << rowPointers[i] << endl;
        for (int j = 0; j < numCols; j++)
        {
            if (denseMatrix[i][j] != 0)
            {
                columnIndices[k] = j;
                values[k] = denseMatrix[i][j];
                k++;
            }
        }
    }
    rowPointers[numRows] = k;

    return;
}

int main()
{

    int n = 0;

    int arr[4] = {1, 1, 10, 9};
    int nValues[4] = {5,10, 20, 400};
    int increments[4] = {0,0, 20, 200};

    for (size_t ii = 0; ii < 4; ii++)
    {
        n = nValues[ii];

        for (size_t m = 0; m < arr[ii]; m++)
        {

            vector<vector<vector<int>>> twoSparseMatrix = twoRandomSparseMatrix(n);

            // // print the two matrixs
            // for (int k = 0; k < 2; k++)
            // {
            //     cout << "Matrix " << k + 1 << ":" << endl;
            //     for (int i = 0; i < n; i++)
            //     {
            //         for (int j = 0; j < n; j++)
            //         {
            //             cout << twoSparseMatrix[k][i][j] << " ";
            //         }
            //         cout << endl;
            //     }
            //     cout << endl;
            // }

            // convert the two matrixs to sparse matrixs
            vector<int> valuesa;
            vector<int> columnIndicesa;
            vector<int> rowPointersa;
            vector<int> valuesb;
            vector<int> columnIndicesb;
            vector<int> rowPointersb;

            denseToSparse(twoSparseMatrix[0], valuesa, columnIndicesa, rowPointersa);
            denseToSparse(twoSparseMatrix[1], valuesb, columnIndicesb, rowPointersb);

            // write both the sparse matrixs to a single file in the format:
            // n
            // values1
            // columnIndices1
            // rowPointers1
            // values2
            // columnIndices2
            // rowPointers2

            // open the file
            string filename = "testSample_p2c/in_" + to_string(n) + ".txt";
            ofstream file(filename);

            // check if the file is open
            if (!file.is_open())
            {
                cout << "Could not open the file" << endl;
                return 1;
            }

            // write the order of matrix
            file << n << endl;

            // write the values of the first matrix
            // for (size_t ab = 0; ab < 2; ab++)

            // {
            for (int i = 0; i < valuesa.size(); i++)
            {
                file << valuesa[i] << " ";
            }
            file << endl;

            // write the columnIndices of the first matrix
            for (int i = 0; i < columnIndicesa.size(); i++)
            {
                file << columnIndicesa[i] << " ";
            }
            file << endl;

            // write the rowPointers of the first matrix
            for (int i = 0; i < rowPointersa.size(); i++)
            {
                file << rowPointersa[i] << " ";
            }
            file << endl;

            for (int i = 0; i < valuesb.size(); i++)
            {
                file << valuesb[i] << " ";
            }
            file << endl;

            // write the columnIndices of the first matrix
            for (int i = 0; i < columnIndicesb.size(); i++)
            {
                file << columnIndicesb[i] << " ";
            }
            file << endl;

            // write the rowPointers of the first matrix
            for (int i = 0; i < rowPointersb.size(); i++)
            {
                file << rowPointersb[i] << " ";
            }
            file << endl;

            // }

            // close the file
            file.close();

            n += increments[ii];
        }
    }
    return 0;
}

// int main(int argc, char *argv[])
// {

//     int n = 0;

//     int arr[3] = {1, 10, 9};
//     int nValues[3] = {10, 20, 400};
//     int increments[3] = {0, 20, 200};

//     for (size_t ii = 0; ii < 3; ii++)
//     {
//         n = nValues[ii];

//         for (size_t m = 0; m < arr[ii]; m++)
//         {

//             // open the file
//             string filename = "testSample_p2c/in_" + to_string(n) + ".txt";
//             ofstream file(filename);

//             // check if the file is open
//             if (!file.is_open())
//             {
//                 cout << "Could not open the file" << endl;
//                 return 1;
//             }

//             // write the order of matrix
//             file << n << endl;

//             int k = 0;
//             while (k < 2)
//             {

//                 int a = 0;
//                 // generate a random sparse matrix
//                 srand(time(NULL));
//                 for (int i = 0; i < n; i++)
//                 {
//                     for (int j = 0; j < n - 1; j++)
//                     {
//                         a = rand() % 10;
//                         if (a < 5)
//                         {
//                             file << a << " ";
//                         }
//                         else
//                         {
//                             file << 0 << " ";
//                         }
//                     }
//                     file << rand() % 10 << endl;
//                 }

//                 k++;
//             }
//             // close the file
//             file.close(); // write the number of rows and columns

//             // n *= 10;
//             n += increments[ii];
//         }
//     }

//     return 0;
// }
