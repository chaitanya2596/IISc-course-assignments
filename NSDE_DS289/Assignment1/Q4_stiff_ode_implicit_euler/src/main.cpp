#include <iostream>
#include <vector>

#include "headerFiles/ioFiles.h"
#include "headerFiles/schemeSelector.h"
#include "headerFiles/analytical.h"

using namespace std;
typedef double real;

int main()
{
    string schemeName[4] = {"EUE", "RK4", "ABS", "EUI"};
    vector<real> input = readInput();
    real stepSize = input[2];
    int noOfSteps = ((input[1] - input[0]) / stepSize);
    int scheme = input[4];

    // cout << "noOfSteps = " << noOfSteps << endl;

    vector<real> xValues;
    vector<real> yValues;

    // initialization from the given intial value i.e. @t=0, y=1
    cout << endl << "---- Initializing Inputs ----" << endl;
    xValues.push_back(input[0]);
    yValues.push_back(input[3]);

    cout << endl << "---- Solving with " + schemeName[scheme] << " with a time step of " << stepSize << ". ----" << endl;
    vector<vector<real>> solution = schemeSelector(scheme, stepSize, xValues, yValues, noOfSteps);


    cout << endl << "---- analytical solution calculations. ----" << endl;

    
    vector<real> analyticalSol = anaSol(noOfSteps, stepSize);

    string outfileName = schemeName[scheme] + "_" + to_string(stepSize) + ".csv";
    cout << endl << "---- Writting results to a CSV file ----" << endl << endl;
    outputToCsv(noOfSteps, solution[0], solution[1], analyticalSol, outfileName);



    return 0;
}