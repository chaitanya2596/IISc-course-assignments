#include <iostream>
#include <cmath>

using namespace std;
typedef double real;


// a --> t,  b --> y
real functionValue(real a, real b)
{
    real c;
    c = pow(a,2)*b - (1.1*b);
    // cout <<endl;
    // cout << c << " , " << a << " , " << b<<  endl;

    return c;
}