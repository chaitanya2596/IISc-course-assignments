#ifndef MERGESORT_H
#define MERGESORT_H
#include <vector>

// local sort used by every process: top-down merge sort with one scratch buffer allocated up front
void mergeSort(int *array, int n);

// merges the two sorted arrays a (na elements) and b (nb elements) into out
void mergeTwo(const int *a, int na, const int *b, int nb, int *out);

// sequential quick sort, the baseline for the speedups
void quickSort(int *array, int n);

#endif
