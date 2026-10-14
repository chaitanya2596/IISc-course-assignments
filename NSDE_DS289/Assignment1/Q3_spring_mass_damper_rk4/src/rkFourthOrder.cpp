#include <iostream>
#include "headerFiles/XFS.h"
#include "headerFiles/VFS.h"

using namespace std;
typedef double real;

// Rungekutta Method

pair<real, real> rungeKuttaFour(real stepSize, real Xo, real Vo)
{
    real a_X, b_X, c_X, d_X, a_V, b_V, c_V, d_V;
    pair<real, real> aa;

    // cout << "Vo  = " << Vo << " and Yo = " << Xo << endl;

    a_X = stepSize * XFS(Vo);
    a_V = stepSize * VFS(Xo, Vo);

    b_X = stepSize * XFS(Vo + (a_V / 2));
    b_V = stepSize * VFS(Xo + (a_X / 2), Vo + (a_V / 2));

    c_X = stepSize * XFS(Vo + (b_V / 2));
    c_V = stepSize * VFS(Xo + (b_X / 2), Vo + (b_V / 2));

    d_X = stepSize * XFS(Vo + c_V);
    d_V = stepSize * VFS(Xo + c_X, Vo + c_V);



    aa.first = Xo + ((a_X + (2 * b_X) + (2 * c_X) + d_X) / 6);
    aa.second = Vo + ((a_V + (2 * b_V) + (2 * c_V) + d_V) / 6);

    return aa;
}
