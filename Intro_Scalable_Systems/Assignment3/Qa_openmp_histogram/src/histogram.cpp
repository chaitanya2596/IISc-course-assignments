#include <vector>
#include <omp.h>

#include "histogram.h"

using namespace std;

void hist_serial(const vector<int> &input, int interval, vector<int> &histogram)
{
    int n = input.size();
    int *h = histogram.data();
    const int *in = input.data();
    for (int j = 0; j < n; j++)
    {
        h[(in[j] - 1) / interval]++;
    }
}

void hist_atomic(const vector<int> &input, int interval, vector<int> &histogram)
{
    int n = input.size();
    int *h = histogram.data();
    const int *in = input.data();
#pragma omp parallel for schedule(static)
    for (int j = 0; j < n; j++)
    {
#pragma omp atomic
        h[(in[j] - 1) / interval]++;
    }
}

void hist_critical(const vector<int> &input, int interval, vector<int> &histogram)
{
    int n = input.size();
    int *h = histogram.data();
    const int *in = input.data();
#pragma omp parallel for schedule(static)
    for (int j = 0; j < n; j++)
    {
#pragma omp critical
        h[(in[j] - 1) / interval]++;
    }
}

void hist_local(const vector<int> &input, int interval, vector<int> &histogram)
{
    int n = input.size();
    int bins = histogram.size();
    int *h = histogram.data();
    const int *in = input.data();
#pragma omp parallel
    {
        // private counts on the stack of the thread; padded so that two threads never share a cache line
        int local_freq[64 + 32] = {0};
        int *lf = local_freq + 16;

#pragma omp for schedule(static) nowait
        for (int j = 0; j < n; j++)
        {
            lf[(in[j] - 1) / interval]++;
        }

#pragma omp critical
        {
            for (int b = 0; b < bins; b++)
            {
                h[b] += lf[b];
            }
        }
    }
}
