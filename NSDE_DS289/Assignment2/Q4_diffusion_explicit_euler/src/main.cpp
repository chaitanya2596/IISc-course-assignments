#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>

#include "headerFiles/ioFiles.h"
#include "headerFiles/initialCondition.h"
#include "headerFiles/analytical.h"
#include "headerFiles/error.h"

using namespace std;
typedef double real;

// u_t = alpha u_xx on a periodic domain of length 2 pi, explicit Euler in time, second order
// central difference in space, at a fixed stability parameter rd = alpha dt / dx^2.
// Usage: ./diffusion [rd]      (rd defaults to the value in input/input.txt)

int main(int argc, char **argv)
{

     vector<real> input = readInput();
     int npx = input[1];
     real rd = input[0];
     if (argc > 1)
          rd = atof(argv[1]);
     vector<pair<real, real>> errorlog;
     vector<real> errorFullSteps; // error at the last full step (t = nSteps * dt), i.e. with rd exactly constant

     for (size_t ii = 1; ii <= 4; ii++)
     {
          if (ii != 1)
          {
               npx = npx * 2;
          }

          real alpha = input[2];
          real tFinal = input[3];

          // domain Length. The domain is periodic, so the npx points are x_i = i * dx with dx = L / npx
          // (the point x = L is the same point as x = 0 and is not stored twice).
          real dlength = 8 * atan(1);
          real dx = dlength / npx;
          real dt = (rd * pow(dx, 2)) / alpha;

          // number of full steps at the requested rd, and the short last step that lands exactly on tFinal
          long nSteps = (long)floor(tFinal / dt + 1e-9);
          real dtLast = tFinal - nSteps * dt;
          if (dtLast < 1e-14)
               dtLast = 0;

          cout << "---- rd = " << rd << "  grid points = " << npx << "  dx = " << dx << "  dt = " << dt
               << "  steps = " << nSteps << " + last step of " << dtLast << " ----" << endl;

          vector<real> u = initialCondition(npx, dx, dlength);
          vector<real> nxtTime(npx);

          for (long n = 0; n < nSteps; n++)
          {
               for (int j = 0; j < npx; j++)
               {
                    int jm = (j == 0) ? npx - 1 : j - 1;
                    int jp = (j == npx - 1) ? 0 : j + 1;
                    nxtTime[j] = (rd * u[jp]) + (u[j] * (1 - (2 * rd))) + (rd * u[jm]);
               }
               u = nxtTime;
          }

          errorFullSteps.push_back(avgError(anaSol(alpha, nSteps * dt, dx, npx), u));

          if (dtLast > 0)
          {
               real frd = alpha * dtLast / pow(dx, 2);
               for (int j = 0; j < npx; j++)
               {
                    int jm = (j == 0) ? npx - 1 : j - 1;
                    int jp = (j == npx - 1) ? 0 : j + 1;
                    nxtTime[j] = (frd * u[jp]) + (u[j] * (1 - (2 * frd))) + (frd * u[jm]);
               }
               u = nxtTime;
          }

          // analytical solution at the final time
          vector<real> analyticalSolution = anaSol(alpha, tFinal, dx, npx);

          // final profile: x, numerical, analytical
          ofstream prof;
          prof.open("output/" + to_string(npx) + "_" + to_string(rd) + ".csv");
          prof << setprecision(15) << "x,numerical,analytical" << endl;
          for (int j = 0; j < npx; j++)
          {
               prof << j * dx << "," << u[j] << "," << analyticalSolution[j] << endl;
          }
          prof.close();

          // average error calcualtions
          pair<real, real> error;
          error.first = npx;
          error.second = avgError(analyticalSolution, u);
          errorlog.push_back(error);
     }

     ofstream err;
     err.open("output/error_rd_" + to_string(rd) + ".csv");
     err << setprecision(15) << "GridSize,avgerror_at_tend,order,avgerror_at_last_full_step,order" << endl;
     cout << endl << "rd = " << rd << endl;
     for (size_t i = 0; i < 4; i++)
     {
          real order = (i == 0) ? 0 : log(errorlog[i - 1].second / errorlog[i].second) / log(2.0);
          real orderFull = (i == 0) ? 0 : log(errorFullSteps[i - 1] / errorFullSteps[i]) / log(2.0);
          err << errorlog[i].first << "," << errorlog[i].second << "," << order << "," << errorFullSteps[i] << "," << orderFull << endl;
          cout << "N = " << errorlog[i].first << "   E(N) at t_end = " << errorlog[i].second << "  order = " << order
               << "   |   E(N) at last full step = " << errorFullSteps[i] << "  order = " << orderFull << endl;
     }
     err.close();
}
