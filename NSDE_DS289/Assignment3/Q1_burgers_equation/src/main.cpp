#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include "headerFiles/ioFiles.h"
#include "headerFiles/initialCondition.h"

using namespace std;
typedef double real;

// Viscous Burgers equation u_t + u u_x = alpha u_xx on a periodic domain of length 1.
//   scheme 0 --> explicit Euler + first order upwind for u u_x (no viscous term used when alpha = 0)
//   scheme 1 --> explicit Euler + second order central difference for u u_x and u_xx
//   scheme 2 --> explicit Euler + first order upwind on the non-conservative form u u_x (for comparison)
// Usage: ./burgersEquation [grid_points alpha scheme]    (defaults come from input/input.txt)

// one explicit Euler step of size dt
vector<real> oneStep(const vector<real> &u, int scheme, real dt, real dx, real alpha)
{
     int npx = u.size();
     real rd = dt / dx;
     real rc = (alpha * dt) / (pow(dx, 2));
     vector<real> nxtTime(npx);

     for (int j = 0; j < npx; j++)
     {
          int jm = (j == 0) ? npx - 1 : j - 1;
          int jp = (j == npx - 1) ? 0 : j + 1;
          if (scheme == 0)
          {
               // first order upwind on the conservative form u_t + (u^2/2)_x = 0.
               // The flux through a cell face is taken from the side the wave comes from:
               //   F(j+1/2) = max(u_j, 0)^2 / 2 + min(u_j+1, 0)^2 / 2
               // In regions where u keeps one sign this is exactly the backward (u > 0) or forward
               // (u < 0) difference of u^2/2, and because every face flux is used by both neighbours
               // the integral of u is conserved, so shocks travel at the correct speed.
               real fRight = pow(max(u[j], 0.0), 2) / 2 + pow(min(u[jp], 0.0), 2) / 2;
               real fLeft = pow(max(u[jm], 0.0), 2) / 2 + pow(min(u[j], 0.0), 2) / 2;
               nxtTime[j] = u[j] - rd * (fRight - fLeft);
               nxtTime[j] += rc * (u[jp] - 2 * u[j] + u[jm]);
          }
          else if (scheme == 2)
          {
               // upwind on the non-conservative form u_t + u u_x = 0. A point where u = 0 never
               // changes in this form, so shocks stay at the zeros of the initial condition.
               if (u[j] >= 0)
               {
                    nxtTime[j] = u[j] - rd * u[j] * (u[j] - u[jm]);
               }
               else
               {
                    nxtTime[j] = u[j] - rd * u[j] * (u[jp] - u[j]);
               }
               nxtTime[j] += rc * (u[jp] - 2 * u[j] + u[jm]);
          }
          else
          {
               nxtTime[j] = u[j] - (rd * u[j] / 2) * (u[jp] - u[jm]) + rc * (u[jp] - 2 * u[j] + u[jm]);
          }
     }
     return nxtTime;
}

int main(int argc, char **argv)
{

     vector<real> input = readInput();
     real dt = input[0];
     int npx = input[1];
     real alpha = input[2];
     real tFinal = input[3];
     int scheme = input[4];
     if (argc > 3)
     {
          npx = atoi(argv[1]);
          alpha = atof(argv[2]);
          scheme = atoi(argv[3]);
     }
     string name[3] = {"Eu_upwind", "Eu_CD2", "Eu_upwind_nonconservative"};

     // domain Length. Periodic, so the npx points are x_i = i * dx with dx = L / npx
     real dlength = 1;
     real dx = dlength / npx;

     vector<real> u = initialCondition(npx, dx, dlength);
     real umax0 = 0;
     for (int j = 0; j < npx; j++)
          umax0 = max(umax0, fabs(u[j]));

     long nSteps = (long)floor(tFinal / dt + 1e-9);
     real dtLast = tFinal - nSteps * dt;
     if (dtLast < 1e-14)
          dtLast = 0;

     cout << "---- " << name[scheme] << "  grid points = " << npx << "  dx = " << dx << "  dt = " << dt << "  alpha = " << alpha << " ----" << endl;
     cout << "max |u| at t = 0 : " << umax0 << "   convective number max|u| dt/dx = " << umax0 * dt / dx
          << "   diffusion number alpha dt/dx^2 = " << alpha * dt / (dx * dx) << endl;

     // solution is stored every 'every' steps (and at the final time) for plotting
     int every = 25;
     vector<real> times;
     vector<vector<real>> solution;
     times.push_back(0);
     solution.push_back(u);

     for (long n = 1; n <= nSteps; n++)
     {
          u = oneStep(u, scheme, dt, dx, alpha);
          if (n % every == 0)
          {
               times.push_back(n * dt);
               solution.push_back(u);
          }
     }
     if (dtLast > 0)
     {
          u = oneStep(u, scheme, dtLast, dx, alpha);
     }
     times.push_back(tFinal);
     solution.push_back(u);

     // position of the first shock (steepest drop in 0 < x < 0.4) and the integral of u (conserved, = 0)
     int jShock = 0;
     real steep = 0, mass = 0;
     for (int j = 0; j < npx; j++)
     {
          mass += u[j] * dx;
          int jp = (j == npx - 1) ? 0 : j + 1;
          if (j * dx < 0.4 && u[j] - u[jp] > steep)
          {
               steep = u[j] - u[jp];
               jShock = j;
          }
     }
     cout << "first shock at x = " << (jShock + 0.5) * dx << "   integral of u = " << mass << endl;

     real umin = *min_element(u.begin(), u.end());
     real umax = *max_element(u.begin(), u.end());
     bool finite = true;
     for (int j = 0; j < npx; j++)
          if (!(fabs(u[j]) < 1e300))
               finite = false;
     cout << "steps = " << nSteps << " + last step of " << dtLast << "   at t = " << tFinal << " : min u = " << umin << "  max u = " << umax
          << (finite ? "" : "   (solution blew up)") << endl;

     // rows = x, columns = stored times
     char aname[32];
     sprintf(aname, "%g", alpha);
     string outfileName = "output/" + to_string(npx) + "_alpha" + aname + "_" + name[scheme] + ".csv";
     ofstream out(outfileName);
     out << setprecision(10) << "x";
     for (size_t k = 0; k < times.size(); k++)
          out << ",t=" << times[k];
     out << endl;
     for (int j = 0; j < npx; j++)
     {
          out << j * dx;
          for (size_t k = 0; k < times.size(); k++)
               out << "," << solution[k][j];
          out << endl;
     }
     out.close();
     cout << "written " << outfileName << endl << endl;
}
