#ifndef USER_CODE_H
#define USER_CODE_H

#include <vector>
#include <string>
#include <utility>
#include <map>
#include <unordered_map>
#include <stdio.h>
#include <iostream>
#include <sys/resource.h> 
#include <algorithm>
#include <unistd.h>

using namespace std;

// ############################################################# QUESTION 1A #############################################################






// Function to convert a dense matrix to a sparse matrix representation
void denseToSparse(const std::vector<std::vector<int>> &denseMatrix,
                   std::vector<int> &values,
                   std::vector<int> &columnIndices,
                   std::vector<int> &rowPointers)
{
    // Write your code here
    int numRows = denseMatrix.size();
    int numCols = denseMatrix[0].size();


    // get the number of non-zero elements
    int numNonZero = 0;
    for (int i = 0; i < numRows; i++)
    {
        numNonZero += count_if(denseMatrix[i].begin(), denseMatrix[i].end(), [](int x)
                               { return x != 0; });
        // cout << "number of non-zero elements = " << numNonZero << endl;
    }

    if (numNonZero == 0)
    {
        // no non-zero values: empty value and column arrays, every row pointer is 0
        values.clear();
        columnIndices.clear();
        rowPointers.assign(numRows + 1, 0);
        return;
    }

    // resize the vectors
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

// ############################################################# QUESTION 2A #############################################################

// Function to perform dense matrix multiplication

void denseMatrixMultiplication(const std::vector<std::vector<int>> &denseMatrix1,
                               const std::vector<std::vector<int>> &denseMatrix2,
                               int N,
                               std::vector<std::vector<int>> &denseMatrix_result)
{
    // // print a 2D matrix
    // for (int i = 0; i < denseMatrix1.size(); i++)
    // {
    //     for (int j = 0; j < denseMatrix1[0].size(); j++)
    //     {
    //         cout << denseMatrix1[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    // cout << "im second matrix" << endl;
    // for (int i = 0; i < denseMatrix2.size(); i++)
    // {
    //     for (int j = 0; j < denseMatrix2[0].size(); j++)
    //     {
    //         cout << denseMatrix2[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    // Write your code here
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            denseMatrix_result[i][j] = 0;
            for (int k = 0; k < N; k++)
            {
                denseMatrix_result[i][j] += denseMatrix1[i][k] * denseMatrix2[k][j];
            }
        }
    }

    return;
}

// ############################################################# QUESTION 2B #############################################################

// Function to perform dense matrix multiplication using block matrix multiplication
void blockMatrixMultiplication(const std::vector<std::vector<int>> &denseMatrix1,
                               const std::vector<std::vector<int>> &denseMatrix2,
                               int N, int B,
                               std::vector<std::vector<int>> &denseMatrix_result)
{

    if (B == 0 || N == 0)
    {
        cout << "B or N cannot be zero" << endl;
        return;
    }
    else if (B > N)
    {
        B = N; // one block covers the whole matrix
    }
    

    // Write your code here
    int sum;
    for (int ii = 0; ii < N; ii += B)
    {
        for (int jj = 0; jj < N; jj += B)
        {
            for (int kk = 0; kk < N; kk += B)
            {
                // cout << "Block " << ii << " " << jj << " " << kk << endl;
                for (int i = ii; i < min(ii + B, N); i++)
                {
                    for (int j = jj; j < min(jj + B, N); j++)
                    {
                        sum = 0;
                        for (int k = kk; k < min(kk + B, N); k++)
                        {
                            sum += denseMatrix1[i][k] * denseMatrix2[k][j];
                            // cout << sum << " = " << denseMatrix1[i][k] << " * " << denseMatrix2[k][j] << endl;
                        }
                        denseMatrix_result[i][j] += sum;
                    }
                }
            }
        }
    }

    return;
}

// ############################################################# QUESTION 2C #############################################################

// Function to perform sparse matrix multiplication
void sparseMatrixMultiplication(const std::vector<int> &values1,
                                const std::vector<int> &columnIndices1,
                                const std::vector<int> &rowPointers1,
                                const std::vector<int> &values2,
                                const std::vector<int> &columnIndices2,
                                const std::vector<int> &rowPointers2,
                                int N,
                                std::vector<int> &valuesResult,
                                std::vector<int> &columnIndicesResult,
                                std::vector<int> &rowPointersResult)
{
    // Write your code here

    rowPointersResult.clear();
    valuesResult.clear();
    columnIndicesResult.clear();
    rowPointersResult.push_back(0);
    std::vector<int> row_vals(N, 0); // one dense row of the result
    for (int row = 0; row < N; row++)
    {
        std::fill(row_vals.begin(), row_vals.end(), 0);
        int row_point_start = rowPointers1[row];
        int row_point_end = rowPointers1[row + 1];
        for (int i = row_point_start; i < row_point_end; i++)
        {
            int col_point_start = rowPointers2[columnIndices1[i]];
            int col_point_end = rowPointers2[columnIndices1[i] + 1];

            for (int j = col_point_start; j < col_point_end; j++)
            {
                row_vals[columnIndices2[j]] += values1[i] * values2[j];
            }
        }
        for (int rc = 0; rc < N; rc++)
        {
            if (row_vals[rc] == 0)
                continue;
            else
            {
                valuesResult.push_back(row_vals[rc]);
                columnIndicesResult.push_back(rc);
            }
        }
        rowPointersResult.push_back(valuesResult.size());
    }

    return;
}

// ############################################################# QUESTION 3a #############################################################

// function For finding patterns in graphs using a map
void findTrianglesAsMap(const std::vector<std::vector<int>> &adjMat, int N, std::map<std::vector<int>, int> &outputMap)
{
    // Write your code here

    // map<vector<int>, int> outputMap;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (adjMat[i][j] > 0)
            {
                for (int k = 0; k < N; k++)
                {
                    if (adjMat[j][k] > 0 && adjMat[k][i] > 0)
                    {

                        std::vector<int> pattern = {adjMat[i][j], adjMat[j][k], adjMat[k][i]};

                        // Use [] operator to access and update outputMap
                        outputMap[pattern]++;
                    }
                }
            }
        }
    }


    return;
}

// ############################################################# QUESTION 3b #############################################################

// function For finding patterns in graphs using a vector

void findTrianglesAsVec(const std::vector<std::vector<int>> &adjMat, int N, std::vector<std::vector<int>> &outputVec)
{
    // Write your code here


    vector<vector<int>> patterns;
    vector<int> pattern;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (adjMat[i][j] > 0)
            {
                for (int k = 0; k < N; k++)
                {
                    if (adjMat[j][k] > 0 && adjMat[k][i] > 0)
                    {

                        pattern = {adjMat[i][j], adjMat[j][k], adjMat[k][i]};

                        // searching for the pattern in the vector
                        auto it = find(patterns.begin(), patterns.end(), pattern);

                        // if the pattern is found, increment the frequency
                        if (it != patterns.end())
                        {
                            int index = it - patterns.begin();
                            outputVec[index][3]++;
                        }
                        // if the pattern is not found, add it to the vector
                        else
                        {
                            patterns.push_back(pattern);
                            pattern.push_back(1);
                            outputVec.push_back(pattern);
                        }
                    }
                }
            }
        }
    }


    return;
}

#endif // USER_CODE_H




