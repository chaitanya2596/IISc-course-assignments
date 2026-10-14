#include <iostream>
#include <cmath>
#include <vector>

using namespace std;
typedef double real;

// Thomas algorithm for a tridiagonal system
// p --> lower diagonal, q --> main diagonal, r --> upper diagonal, o --> right hand side, n --> number of unknowns
vector<real> tridiagonal_solver(vector<real> p, vector<real> q, vector<real> r, vector<real> o, int n)
{
    vector<real> rs, os;
    vector<real> T(n, 0.0);

    rs.push_back(r[0] / q[0]);
    os.push_back(o[0] / q[0]);

    for (int i = 1; i < n; i++)
    {
        rs.push_back(r[i] / (q[i] - p[i] * rs[i - 1]));
        os.push_back((o[i] - (p[i] * os[i - 1])) / (q[i] - (p[i] * rs[i - 1])));
    }

    T[n - 1] = os[n - 1];
    for (int i = n - 2; i >= 0; i--)
    {
        T[i] = os[i] - rs[i] * T[i + 1];
    }

    return T;
}
