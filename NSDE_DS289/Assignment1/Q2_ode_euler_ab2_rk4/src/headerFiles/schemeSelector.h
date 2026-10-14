#ifndef SCHEMESELECTOR_H
#define SCHEMESELECTOR_H

#include <iostream>
#include <vector>
#include <cmath>

#include "functionSolver.h"

using namespace std;
typedef double real;

vector<vector<real>> schemeSelector(int a, real stepSize, vector<real> Xo, vector<real> Yo, int nos);

#endif