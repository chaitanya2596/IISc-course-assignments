#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
typedef double real;

real pie = 4 * atan(1);

// u(x,0) = sin(4 pi x) + sin(6 pi x) + sin(10 pi x) on the npx periodic grid points x_i = i * dx
vector<real> initialCondition(int npx, real dx, real length)
{
    vector<real> icon;
    for (int ic = 0; ic < npx; ic++)
    {
        icon.push_back(sin(4 * pie * ic * dx) + sin(6 * ic * pie * dx) + sin(10 * ic * pie * dx));
    }
    return icon;
}
