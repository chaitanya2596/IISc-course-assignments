#include <iostream>
// #include <fstream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <string>
// #include <string.h>
// #include <sstream>


#include "headerFiles/tdma.h"
#include "headerFiles/ioFiles.h"



using namespace std;
typedef double real;



int main(int argc, char **argv)
{
    vector<real> p, q, r, o;
    int num_points = 1025;
    int S = 1500;
    // optional: ./tdma <S> <num_points>
    if (argc > 1)
        S = atoi(argv[1]);
    if (argc > 2)
        num_points = atoi(argv[2]);

    real stepSize = 1.0 / (num_points - 1);



    cout << endl << "------ Initializing the Input ------" << endl;
    p.push_back(0);
    q.push_back(-1.0 / stepSize);
    r.push_back(1.0 / stepSize) ;
    o.push_back(0);
   

    for (int i = 1; i < num_points - 1; i++)
    {
        double rPoint = i * stepSize;
        o.push_back(-S);
        p.push_back((1.0 / pow(stepSize, 2.0)) - (1.0 / (2.0 * stepSize * rPoint)));
        r.push_back((1.0 / pow(stepSize, 2.0)) + (1.0 / (2.0 * stepSize * rPoint)));
        q.push_back(-2.0 / pow(stepSize, 2.0));
        
    }

    p.push_back(0.0);
    q.push_back(1.0);
    r.push_back(0.0);
    o.push_back(1.0);

    

    cout << endl << "------ Solving the Tridiagonal Matrix ------" << endl;
    vector<real> temp = tridiagonal_solver(p, q, r, o, num_points);


    cout << endl << "------ Writing the Output ------" << endl;
    
    string name = "_" + to_string(S) ;
    outputToCsv(temp.size() - 1, temp, name, stepSize);

    // exact solution of T'' + T'/r = -S with T'(0) = 0, T(1) = 1 is T = 1 + S (1 - r^2) / 4
    real maxErr = 0;
    for (int i = 0; i < num_points; i++)
    {
        real rr = i * stepSize;
        maxErr = max(maxErr, fabs(temp[i] - (1.0 + S * (1.0 - rr * rr) / 4.0)));
    }
    cout << "S = " << S << "  points = " << num_points << "  T(0) = " << temp[0] << "  exact T(0) = " << 1.0 + S / 4.0 << "  max error = " << maxErr << endl;

   

    return 0;
}
