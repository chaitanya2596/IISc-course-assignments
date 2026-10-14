#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
typedef double real;

// nos --> number of steps, ss --> stepsize
vector<real> anaSol(int nos, real ss)
{
    vector<real> anastorage;
    for (int ana = 0; ana < nos + 1; ana++)
    {
        // cout << (exp(-ana*ss))*(1-exp(-199999*ana*ss)) << endl;
        anastorage.push_back((exp(-ana*ss))*(1-exp(-199999*ana*ss)));
    }

    return anastorage;   
    
}