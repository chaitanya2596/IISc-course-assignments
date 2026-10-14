#include <iostream>
#include <cmath>

#include "headerFiles/ioFiles.h"

using namespace std;
typedef double real;


// Function solver for V
// a --> x,  b --> v
real VFS(real a, real b)
{
    // will read parameters in this order 0
    // m --> mass, k --> stiffness of spring, c --> damping coeff


    vector <real> sp = readInput("input/sytstemParameters.txt");

    real v = - (((sp[1]/sp[0])*a)  +  ((sp[2]/sp[0])*b));
    
    


    // cout <<endl;
    // cout << c << " , " << a << " , " << b<<  endl;

    return v;
}