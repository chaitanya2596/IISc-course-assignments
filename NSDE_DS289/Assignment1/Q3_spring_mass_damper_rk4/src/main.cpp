#include <iostream>
#include <vector>
#include <cmath>

#include "headerFiles/ioFiles.h"
#include "headerFiles/rkFourthOrder.h"

using namespace std;

typedef double real;

int main()
{

    vector<real> input = readInput("input/input.txt");
    vector <real> para = readInput("input/sytstemParameters.txt");
    for (size_t ii = 0; ii < 6; ii++)
    {


        cout << endl <<  "------ Initializing Inputs ------" << endl << endl;

        // initializing inputs
        real start = input[0];
        real end = input[1];
        // real stepSize = input[2];
        real stepSize = input[2] / (pow(2, ii));
        int noOfSteps = ((end - start) / stepSize);

        // cout << "noOfSteps = " << noOfSteps << endl;

        vector<real> tValues;
        vector<real> xValues;
        vector<real> vValues;

        cout << "------ Initializing intial Conditions ------" << endl << endl;
        // initialization from the given intial value i.e. @t=0, x=1, v=0
        xValues.push_back(input[3]);
        vValues.push_back(input[4]);
        tValues.push_back(start);

       
        cout << "------ Computing with RK4 Scheme with " + to_string(stepSize) + " ------" << endl << endl;
        for (int itt = 1; itt <= noOfSteps; itt++)
        {
            pair<real, real> eachStep = rungeKuttaFour(stepSize, xValues[itt - 1], vValues[itt - 1]);

            tValues.push_back(itt * stepSize);
            xValues.push_back(eachStep.first);
            vValues.push_back(eachStep.second);

           
        }


        cout << "------ Writing results in CSV file ------" << endl << endl <<endl << endl;
        string X = to_string(para[2])  + "_" + to_string(stepSize);

        outputToCsv(noOfSteps, tValues, xValues, vValues, X);
    }

    return 0;
}