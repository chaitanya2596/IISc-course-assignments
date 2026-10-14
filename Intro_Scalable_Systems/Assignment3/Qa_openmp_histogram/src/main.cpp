#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <omp.h>

#include "histogram.h"

using namespace std;

// Histogram of 2,00,000 integers (1..50) for intervals 2, 5, 10, 25.
// For every interval: sequential version, then three OpenMP versions (atomic, critical, private
// per-thread counts) for 2, 4, 8, 16, 32 threads. Every result is compared with the sequential one.
//
// Timing: one histogram of 2 lakh numbers takes well under a millisecond, which is about the same
// as the noise of the timer and of starting the threads. So one "run" repeats the histogram REPS
// times and reports the time of one histogram (total / REPS); as asked, 5 runs are averaged.

typedef void (*hist_fn)(const vector<int> &, int, vector<int> &);

// average time (in microseconds) of one call of fn, over "runs" runs of "reps" repetitions each
double timeIt(hist_fn fn, const vector<int> &input, int interval, int bins, int runs, int reps, vector<int> &result)
{
    double total = 0;
    for (int r = 0; r < runs; r++)
    {
        double start = omp_get_wtime();
        for (int k = 0; k < reps; k++)
        {
            vector<int> histogram(bins, 0);
            fn(input, interval, histogram);
            if (k == reps - 1)
                result = histogram;
        }
        total += (omp_get_wtime() - start) / reps;
    }
    return 1e6 * total / runs;
}

int main(int argc, char **argv)
{
    int distinct_values = 50;
    int runs = 5;
    int reps = (argc > 1) ? atoi(argv[1]) : 200;
    vector<int> histogram_interval = {2, 5, 10, 25};
    vector<int> parallel_threads = {2, 4, 8, 16, 32};

    // read input from file
    vector<int> input;
    ifstream input_file("input/input.txt");
    string temp;
    while (getline(input_file, temp, ','))
    {
        input.push_back(stoi(temp));
    }
    int n = input.size();
    cout << "numbers read: " << n << ", runs averaged: " << runs << ", repetitions per run: " << reps
         << ", cores available: " << omp_get_num_procs() << endl;

    const char *names[3] = {"atomic", "critical", "local"};
    hist_fn fns[3] = {hist_atomic, hist_critical, hist_local};

    ofstream csv("output/timings.csv");
    csv << "interval,method,threads,time_us,speedup,correct" << endl;
    ofstream histfile("output/histograms.txt");
    bool allMatch = true;

    for (size_t i = 0; i < histogram_interval.size(); i++)
    {
        int interval = histogram_interval[i];
        int bins = distinct_values / interval;

        vector<int> serial_sol;
        omp_set_num_threads(1);
        double tSerial = timeIt(hist_serial, input, interval, bins, runs, reps, serial_sol);
        csv << interval << ",serial,1," << tSerial << ",1,yes" << endl;
        cout << endl << "interval " << interval << " (" << bins << " bins): sequential " << tSerial << " us" << endl;

        histfile << "Histogram for " << interval << " histogram interval" << endl;
        long sum = 0;
        for (int b = 0; b < bins; b++)
        {
            histfile << serial_sol[b] << "   ";
            sum += serial_sol[b];
        }
        histfile << endl << "(sum of counts = " << sum << ")" << endl;

        for (int m = 0; m < 3; m++)
        {
            for (size_t q = 0; q < parallel_threads.size(); q++)
            {
                omp_set_num_threads(parallel_threads[q]);
                // the critical version is ~100x slower than the others, fewer repetitions are enough
                int r = (m == 1) ? max(1, reps / 50) : reps;
                vector<int> parallel_sol;
                double t = timeIt(fns[m], input, interval, bins, runs, r, parallel_sol);
                bool match = (parallel_sol == serial_sol);
                allMatch = allMatch && match;
                csv << interval << "," << names[m] << "," << parallel_threads[q] << "," << t << "," << tSerial / t << "," << (match ? "yes" : "NO") << endl;
                cout << "   " << names[m] << "  threads " << parallel_threads[q] << ": " << t << " us   speedup " << tSerial / t
                     << (match ? "" : "   RESULT DOES NOT MATCH") << endl;
            }
        }
    }

    cout << endl << (allMatch ? "Serial and parallel solutions match" : "Serial and parallel solutions do not match") << endl;
    return allMatch ? 0 : 2;
}
