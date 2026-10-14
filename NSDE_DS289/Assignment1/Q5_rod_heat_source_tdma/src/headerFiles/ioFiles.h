#ifndef IOFILES_H
#define IOFILES_H

#include <iostream>
#include <vector>
#include <cmath>
#include <string> 
#include <fstream> 


typedef double real;
using namespace std;

vector<real> readInput(string fileName);
void outputToCsv(int noOfSteps, vector<real> t, string S, real stepsize);

#endif