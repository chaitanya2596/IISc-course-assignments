#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <fstream>

#include "headerFiles/ioFiles.h"
#include "headerFiles/schemeSelector.h"
#include "headerFiles/analytical.h"
#include "headerFiles/error.h"

using namespace std;
typedef double real;

int main(int argc, char **argv)
{

    int reductionInStepsize = 5;
    vector<real> avgerr;
    vector<real> maxerr;
    vector<real> input = readInput();
    string schemeName[3] = {"EU", "RK4", "ABS"};
    int scheme = input[4];
    // optional: ./numericalSolver <scheme> overrides the scheme given in input.txt
    if (argc > 1)
        scheme = atoi(argv[1]);

    for (int s = 0; s < reductionInStepsize; s++)
    {

        real stepSize = input[2] / (pow(2, s));
        real start = input[0];
        real end = input[1];
        int noOfSteps = ((end - start) / stepSize);

        // cout << "noOfSteps = " << noOfSteps << endl;

        vector<real> tValues;
        vector<real> yValues;

        // initialization from the given intial value i.e. @t=0, y=1
        cout << endl
             << "---- Initializing Inputs ----" << endl;
        tValues.push_back(start);
        yValues.push_back(input[3]);



        //computing for a particular scheme    
        cout << endl
             << "---- Solving with " + schemeName[scheme] << " with a time step of " << stepSize << ". ----" << endl;
        vector<vector<real>> solution = schemeSelector(scheme, stepSize, tValues, yValues, noOfSteps);


        
        //computing analytical solution 
        cout << endl
             << "---- analytical solution calculations. ----" << endl;

        vector<real> analyticalSol = anaSol(noOfSteps, stepSize);



        //writing output to CSV file
        string outfileName = schemeName[scheme] + "_" + to_string(stepSize) + ".csv";
        cout << endl
             << "---- Writting results to a CSV file ----" << endl
             << endl;
        outputToCsv(noOfSteps, solution[0], solution[1], analyticalSol, outfileName);

        // computing MAx and Avg error   
        maxerr.push_back(maxError(analyticalSol, solution[1]));
        avgerr.push_back(avgError(analyticalSol, solution[1]));
    }


     // Writting error values to CSV file 
    ofstream fileName;
    fileName.open("output/errorFile" + schemeName[scheme] + ".csv");
    fileName << "stepsize "<< ","<< "Maxerror "<< "," <<"Avgerror " << endl;
    for (size_t i = 0; i < reductionInStepsize; i++)
    {
        fileName << input[2] / (pow(2, i)) << "," << maxerr[i] << "," << avgerr[i] << endl;
    } 
    fileName.close();
}