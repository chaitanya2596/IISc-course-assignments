#ifndef HISTOGRAM_H
#define HISTOGRAM_H
#include <vector>

// value v (1 .. distinct_values) goes to bin (v - 1) / interval, e.g. interval 5: 1-5 -> 0, 6-10 -> 1, ...
void hist_serial(const std::vector<int> &input, int interval, std::vector<int> &histogram);

// shared histogram array, every increment protected with "omp atomic"
void hist_atomic(const std::vector<int> &input, int interval, std::vector<int> &histogram);

// shared histogram array, every increment inside "omp critical"
void hist_critical(const std::vector<int> &input, int interval, std::vector<int> &histogram);

// each thread counts into its own private array, the private arrays are added to the shared
// histogram once at the end (one critical section per thread instead of one per element)
void hist_local(const std::vector<int> &input, int interval, std::vector<int> &histogram);

#endif
