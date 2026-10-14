#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
typedef double real;

// analSol --> analytical Solution, NumSol --> Numerical Solution
real maxError(vector<real> analSol, vector<real> numSol)
{
   
    vector <real> err;
    for (int m = 0; m < analSol.size(); m++)
    {
        // cout << analSol[m] << "-" << numSol[m] << " = " << abs(analSol[m]-numSol[m]) << endl;
        err.push_back(abs(analSol[m]-numSol[m]));
    }
    real maxErrorStorage = *max_element(err.begin(),err.end());

    return maxErrorStorage; 
    
}

real avgError(vector<real> analSol, vector<real> numSol)
{
    
    real sum = 0;
    for (int m = 0; m < analSol.size(); m++)
    {
        // cout << analSol[m] - numSol[m] << endl;
        sum += (abs(analSol[m]-numSol[m]));
    }

    sum = (sum/(analSol.size()));

    

    return sum;   
    
}