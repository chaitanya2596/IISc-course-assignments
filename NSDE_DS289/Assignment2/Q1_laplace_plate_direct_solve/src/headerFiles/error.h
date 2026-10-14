#ifndef ERROR_H
#define ERROR_H

#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
typedef double real;

real maxError(vector<real> analSol, vector<real> numSol);
real avgError(vector<real> analSol, vector<real> numSol);


#endif

