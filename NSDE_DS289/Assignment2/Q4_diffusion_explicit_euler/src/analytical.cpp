#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
typedef double real;

// tf  --> tend or tfinal
// exact solution u(x,t) = exp(-alpha t) sin(x) + exp(-16 alpha t) sin(4x), evaluated at the final time
vector<real> anaSol(real alpha, real tf, real dx, int npx)
{
    vector<real> anastorage;
    for (int ana = 0; ana < npx; ana++)
    {
        real a = ((exp(-alpha * tf)) * sin(dx * ana)) + ((exp(-alpha * 16 * tf)) * sin(4 * dx * ana));
        anastorage.push_back(a);
    }
    return anastorage;
}
