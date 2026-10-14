#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include "headerFiles/ioFiles.h"

using namespace std;
typedef double real;

// Steady heat conduction (Laplace equation) on a rectangular plate, second order central differences.
//   left  : T = tlbc            right : T = trbc
//   bottom: dT/dy = 0           top   : dT/dy = T - 60
// Unknowns are numbered row by row, index = j * npx + i  (i along x, j along y), so the coefficient
// matrix is banded with half band width npx. It is stored in band form and solved directly with
// banded Gaussian elimination (LU without pivoting), so the 128 x 256 grid needs ~70 MB instead of
// the ~8.6 GB a full (npx*npy) x (npx*npy) matrix would need.

int main()
{
     vector<real> input = readInput();
     pair<real, real> xbounds(input[0], input[1]);
     pair<real, real> ybounds(input[2], input[3]);
     int npx = input[4];
     int npy = input[5];
     real trbc = input[6]; // right boundary temperature
     real tlbc = input[7]; // left boundary temperature

     int n = npx * npy;
     int bw = npx;           // half band width
     int width = 2 * bw + 1; // stored entries per row

     real deltax = (xbounds.second - xbounds.first) / (npx - 1);
     real deltay = (ybounds.second - ybounds.first) / (npy - 1);
     real ratio = pow(deltay, 2) / pow(deltax, 2);
     cout << "grid = " << npx << " x " << npy << "  deltax = " << deltax << "  deltay = " << deltay << endl;

     // band[row * width + (col - row + bw)] holds coefficient (row, col)
     vector<real> band((size_t)n * width, 0.0);
     vector<real> bMat(n, 0.0);
     auto A = [&](int row, int col) -> real & { return band[(size_t)row * width + (col - row + bw)]; };

     for (int j = 0; j < npy; j++)
     {
          for (int i = 0; i < npx; i++)
          {
               int c = j * npx + i;
               if (i == 0) // left boundary
               {
                    A(c, c) = 1.0;
                    bMat[c] = tlbc;
               }
               else if (i == npx - 1) // right boundary
               {
                    A(c, c) = 1.0;
                    bMat[c] = trbc;
               }
               else if (j == 0) // bottom boundary, dT/dy = 0 (ghost point T(j-1) = T(j+1))
               {
                    A(c, c - 1) = ratio;
                    A(c, c) = -2.0 * (1 + ratio);
                    A(c, c + 1) = ratio;
                    A(c, c + npx) = 2.0;
               }
               else if (j == npy - 1) // top boundary, dT/dy = T - 60 (ghost point T(j+1) = T(j-1) + 2 dy (T - 60))
               {
                    A(c, c - 1) = ratio;
                    A(c, c) = -2.0 * (1 + ratio - deltay);
                    A(c, c + 1) = ratio;
                    A(c, c - npx) = 2.0;
                    bMat[c] = 120.0 * deltay;
               }
               else // interior points
               {
                    A(c, c - npx) = 1.0;
                    A(c, c - 1) = ratio;
                    A(c, c) = -2.0 * (1 + ratio);
                    A(c, c + 1) = ratio;
                    A(c, c + npx) = 1.0;
               }
          }
     }

     // structure of the coefficient matrix (only written for small grids, it is n x n)
     if (n <= 400)
     {
          ofstream output;
          output.open("output/coeffMat_" + to_string(npx) + "x" + to_string(npy) + ".csv");
          for (int r = 0; r < n; r++)
          {
               for (int c = 0; c < n; c++)
               {
                    output << ((abs(c - r) <= bw) ? A(r, c) : 0.0) << ",";
               }
               output << ",T" << r % npx << "_" << r / npx << "," << bMat[r] << endl;
          }
          output.close();
     }

     // keep a copy of the system to compute the residual after the solve
     vector<real> bandCopy = band;
     vector<real> rhsCopy = bMat;

     // #########################   banded direct solver  ############################
     for (int k = 0; k < n; k++)
     {
          real pivot = A(k, k);
          int last = min(n - 1, k + bw);
          for (int r = k + 1; r <= last; r++)
          {
               real factor = A(r, k) / pivot;
               if (factor == 0.0)
                    continue;
               for (int c = k; c <= last; c++)
               {
                    A(r, c) -= factor * A(k, c);
               }
               bMat[r] -= factor * bMat[k];
          }
     }
     for (int k = n - 1; k >= 0; k--)
     {
          real sum = bMat[k];
          int last = min(n - 1, k + bw);
          for (int c = k + 1; c <= last; c++)
          {
               sum -= A(k, c) * bMat[c];
          }
          bMat[k] = sum / A(k, k);
     }

     // residual max |A x - b| of the original system
     real maxRes = 0;
     for (int r = 0; r < n; r++)
     {
          real sum = 0;
          for (int c = max(0, r - bw); c <= min(n - 1, r + bw); c++)
          {
               sum += bandCopy[(size_t)r * width + (c - r + bw)] * bMat[c];
          }
          maxRes = max(maxRes, fabs(sum - rhsCopy[r]));
     }

     real tmin = *min_element(bMat.begin(), bMat.end());
     real tmax = *max_element(bMat.begin(), bMat.end());
     cout << "max residual |A T - b| = " << maxRes << endl;
     cout << "T min = " << tmin << "  T max = " << tmax << endl;
     cout << setprecision(10) << "T at plate centre = " << bMat[(npy / 2) * npx + npx / 2]
          << "   T at middle of top edge = " << bMat[(npy - 1) * npx + npx / 2] << endl;

     // one row of the file = one y level (bottom to top), columns run along x
     ofstream outputa;
     outputa.open("output/xMat" + to_string(npx) + "x" + to_string(npy) + ".csv");
     outputa << setprecision(10);
     int kkk = 0;
     for (int ik = 0; ik < npy; ik++)
     {
          for (int im = 0; im < npx; im++)
          {
               outputa << bMat[kkk] << ",";
               kkk++;
          }
          outputa << endl;
     }
     outputa.close();
}
