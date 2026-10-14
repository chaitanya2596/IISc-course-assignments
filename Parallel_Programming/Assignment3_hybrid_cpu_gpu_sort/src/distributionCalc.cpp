#include <iostream>
#include <cmath>
#include <algorithm>

#include "distributionCalc.h"

using namespace std;

void distriCalc(long n, int size, int gpuRanks, double gpuWeight, long gpuCapacity,
                vector<long> &distributionArray, vector<long> &displacementarray)
{
    distributionArray.assign(size, 0);
    displacementarray.assign(size, 0);

    long noOfGPUele = 0;
    int cpuRanks = size - gpuRanks;
    if (gpuRanks == 1)
    {
        noOfGPUele = (long)(n * (gpuWeight / (cpuRanks + gpuWeight)));
        noOfGPUele = min(noOfGPUele, gpuCapacity);
        if (cpuRanks == 0)
            noOfGPUele = n;
        distributionArray[size - 1] = noOfGPUele;
    }

    long noOfCPUele = n - noOfGPUele;
    for (int distribu = 0; distribu < cpuRanks; distribu++)
    {
        distributionArray[distribu] = noOfCPUele / cpuRanks + (distribu < noOfCPUele % cpuRanks ? 1 : 0);
    }

    for (int i = 1; i < size; i++)
    {
        displacementarray[i] = displacementarray[i - 1] + distributionArray[i - 1];
    }
}
