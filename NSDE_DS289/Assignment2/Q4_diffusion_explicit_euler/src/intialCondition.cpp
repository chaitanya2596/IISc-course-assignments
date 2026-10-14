#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
typedef double real;

// u(x,0) = sin(4x) + sin(x) on the npx periodic grid points x_i = i * dx, i = 0 .. npx-1
vector<real> initialCondition(int npx, real dx, real length)
{
    vector<real> icon;
    for (int ic = 0; ic < npx; ic++)
    {
        icon.push_back(sin(4 * ic * dx) + sin(ic * dx));
    }
    return icon;
}
