#ifndef IOFILES_H
#define IOFILES_H

#include <iostream>
#include <vector>
#include <cmath>
#include <string> 
#include <fstream> 


typedef double real;
using namespace std;

vector<real> readInput();
void outputToCsv(int noOfSteps, vector<real> dependentVar, vector<real> independentVar,vector<real> analytical, string S);

#endif