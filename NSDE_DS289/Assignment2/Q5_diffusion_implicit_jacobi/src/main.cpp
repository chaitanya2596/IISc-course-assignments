#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <vector>
#include <cstdio>
#include <algorithm>

using namespace std;
// Function to compute analytical solution of the diffusion equation
vector<double> analytical(int N, double dx, double time)
{
    vector<double> sol_vect;
    double x = 0;

    for (int i = 0; i < N; i++)
    {
        double analytic_sol = 0;
        analytic_sol = exp(-24 * time) * sin(4 * x) + exp(-1.5 * time) * sin(x);
        sol_vect.push_back(analytic_sol);
        x = x + dx;
    }

    return sol_vect;
}

// Function to calculate the absolute error between calculated/predicted and analytical solutions
double abs_error(vector<double> analytic, vector<double> predict)
{
    double sum, avg = 0;
    sum = 0;

    for (int i = 0; i < analytic.size(); i++)
    {
        sum += abs(analytic[i] - predict[i]);
    }

    avg = sum / analytic.size();
    return avg;
}

// Function to initialize the temperature field
vector<double> initialize(int N, double dx)
{
    double x = 0.0;
    vector<double> x_list(N, 0);

    for (int i = 0; i < N; i++)
    {
        x_list[i] = sin(4 * x) + sin(x);
        x += dx;
    }

    return x_list;
}

// Matrix Formulation for implicit Euler Scheme
vector<vector<double>> Matrix_Form(int N, double rd)
{
    vector<vector<double>> A(N, vector<double>(N, 0));

    A[0][0] = 1 + 2 * rd;
    A[0][1] = -rd;
    A[0][N - 1] = -rd; // periodic neighbour of point 0

    A[N - 1][N - 1] = 1 + 2 * rd;
    A[N - 1][N - 2] = -rd;
    A[N - 1][0] = -rd; // periodic neighbour of point N-1

    for (int i = 1; i < N - 1; i++)
    {
        A[i][i] = 1 + 2 * rd;
        A[i][i + 1] = -rd;
        A[i][i - 1] = -rd;
    }
    return A;
}

// function based on Jacobi algorithm for Solving Ax=u
// The iteration starts from the previous time level and stops when the largest change of any
// unknown between two sweeps is below tol.
long jacobi_sweeps = 0;
vector<double> jacobi_method(const vector<vector<double>> &A, const vector<double> &u_i_1, double tol)
{

    double err = 100000.0;
    int dim = A.size();
    vector<double> x_i(dim, 0), x_i_1(u_i_1);

    while (err > tol)
    {
        err = 0;
        for (int i = 0; i < dim; i++)
        {
            double sum = 0;
            for (int j = 0; j < dim; j++)
            {
                if (i != j)
                {
                    sum = sum + A[i][j] * x_i_1[j];
                }
            }
            x_i[i] = (u_i_1[i] - sum) / A[i][i];
            err = max(err, fabs(x_i[i] - x_i_1[i]));
        }
        for (int k = 0; k < dim; k++)
        {
            x_i_1[k] = x_i[k];
        }
        jacobi_sweeps++;
    }
    return x_i;
}

// Usage: ./implicitDiffusion [jacobi_tolerance]     (default 1e-4, the value given in the question)
int main(int argc, char **argv)
{
    double tol = 1e-4;
    if (argc > 1)
        tol = atof(argv[1]);

    cout << endl
         << "-------------------------      initilizing problem    -------------------------" << endl
         << endl;
    double t_start = 0.0;
    double t_end = 0.4;
    double L = 8 * atan(1.0);
    vector<double> rd{0.5, 1.0 / 6.0};
    vector<int> N{32, 64, 128, 256};
    vector<vector<double>> A_matrix;
    vector<vector<double>> Error_Matrix(2, vector<double>(4, 0));
    vector<vector<double>> Sweeps_Matrix(2, vector<double>(4, 0));

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            vector<double> analytical_solution;
            int Nx = 0, Nt;
            double dx, dt;
            double x = 0, dt_end, alph = 1.5;
            vector<double> u_i(Nx, 0), u_i_1(Nx, 0);
            Nx = N[j];

            dx = L / Nx; // periodic domain: Nx distinct points, x = L is the same point as x = 0
            dt = (rd[i] * (pow(dx, 2))) / alph;
            Nt = int((t_end - t_start) / dt + 1e-9);
            dt_end = t_end - Nt * dt;

            analytical_solution = analytical(Nx, dx, t_end);

            u_i_1 = initialize(Nx, dx);

            A_matrix = Matrix_Form(Nx, rd[i]);

            for (int t = 0; t < Nt; t++)
            {
                u_i = jacobi_method(A_matrix, u_i_1, tol);
                for (int k = 0; k < Nx; k++)
                {
                    u_i_1[k] = u_i[k];
                }
            }

            // short last step so that the solution lands exactly on t_end
            if (dt_end > 1e-14)
            {
                double rd_end = alph * dt_end / (dx * dx);
                A_matrix.clear();
                A_matrix = Matrix_Form(Nx, rd_end);
                u_i = jacobi_method(A_matrix, u_i_1, tol);
            }
            else
            {
                u_i = u_i_1;
            }
            Sweeps_Matrix[i][j] = jacobi_sweeps;
            jacobi_sweeps = 0;

            Error_Matrix[i][j] = abs_error(analytical_solution, u_i);
        }
    }
    // writing the output in csv file
    char tolName[64];
    sprintf(tolName, "%.0e", tol);
    ofstream f1(string("output/error_implicit_tol_") + tolName + ".csv");
    f1 << scientific; // use scientific notation
    f1 << "rd,N,avgerror,order,jacobi_sweeps" << endl;
    cout << "Jacobi tolerance = " << tol << endl;
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            double order = (j == 0) ? 0 : log(Error_Matrix[i][j - 1] / Error_Matrix[i][j]) / log(2.0);
            f1 << rd[i] << "," << N[j] << "," << Error_Matrix[i][j] << "," << order << "," << Sweeps_Matrix[i][j] << endl;
            cout << "rd = " << rd[i] << "  N = " << N[j] << "  E(N) = " << Error_Matrix[i][j] << "  order = " << order
                 << "  Jacobi sweeps = " << Sweeps_Matrix[i][j] << endl;
        }
    }
    f1.close();

    return 0;
}
