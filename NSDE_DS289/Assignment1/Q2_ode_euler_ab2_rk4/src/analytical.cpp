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
        // cout << exp(((pow((ana*ss),3))/3) - (1.1*(ana*ss))) << endl;
        anastorage.push_back(exp(((pow((ana*ss),3))/3) - (1.1*(ana*ss))));
    }

    return anastorage;   
    
}