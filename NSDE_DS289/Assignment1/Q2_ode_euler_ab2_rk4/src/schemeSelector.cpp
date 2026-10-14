#include <iostream>
#include <vector>
#include <cmath>

#include "headerFiles/functionSolver.h"

using namespace std;
typedef double real;

// a --> funtion selector ( 0 -> Eulers Scheme, 1 -> RungeKutta Scheme, 2 -> Adams Bashforth )
// stepsize
// Xo --> time vector
// Yo --> function vector
// nos --> number of steps
vector<vector<real>> schemeSelector(int a, real stepSize, vector<real> Xo, vector<real> Yo, int nos)
{
    
    real YY;
    vector<vector<real>> computedSolution;

    if (a == 0) // Euler Scheme
    {
        for (int itt = 1; itt <= nos; itt++)
        {
            YY = Yo[itt - 1] + (stepSize * (functionValue(Xo[itt - 1], Yo[itt - 1])));
            Xo.push_back(itt * stepSize);
            Yo.push_back(YY);
        }
        // cout << YY << " = " << Yo << " + " << stepSize << " * " << functionValue(Xo, Yo) << endl;
    }
    else if (a == 1) // Runge Kutta 4
    {
        real YY, a, b, c, d;

        for (int itt = 1; itt <= nos; itt++)
        {

            // cout << "Xo  = " << Xo << " and Yo = " << Yo << endl;

            a = stepSize * functionValue(Xo[itt - 1], Yo[itt - 1]);
            b = stepSize * functionValue(Xo[itt - 1] + (stepSize / 2), Yo[itt - 1] + (a / 2));
            c = stepSize * functionValue(Xo[itt - 1] + (stepSize / 2), Yo[itt - 1] + (b / 2));
            d = stepSize * functionValue(Xo[itt - 1] + stepSize, Yo[itt - 1] + c);

            YY = Yo[itt - 1] + ((a + (2 * b) + (2 * c) + d) / 6);

            Xo.push_back(itt * stepSize);
            Yo.push_back(YY);

            // cout << "b  = " << b << " = "<< stepSize<< "*" << functionValue(Xo + (stepSize/2), Yo + (a/2));
            // cout << "c " << c;
            // cout << "d " << d;

            // cout << Yo <<"+ (1/6)*(" << a  << "+ 2*" << b << " + 2*"<< c << " + "<< d << ")" << endl;
        }
    }
    else if (a == 2) // Adams Bashforth
    {
        for (int itt = 1; itt <= nos; itt++)
        {
            if (itt == 1)
            {
                YY = Yo[itt - 1] + (stepSize * (functionValue(Xo[itt - 1], Yo[itt - 1])));
                Xo.push_back(itt * stepSize);
                Yo.push_back(YY);
            }
            else
            {
                YY = Yo[itt-1] + ((stepSize*((3*functionValue(Xo[itt - 1], Yo[itt - 1]))-(functionValue(Xo[itt - 2], Yo[itt - 2]))))/2) ;
                Xo.push_back(itt * stepSize);
                Yo.push_back(YY);
            }
        }
    }

    computedSolution.push_back(Xo);
    computedSolution.push_back(Yo);

    return computedSolution;
}