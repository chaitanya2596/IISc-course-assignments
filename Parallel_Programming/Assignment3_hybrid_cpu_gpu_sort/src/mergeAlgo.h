#ifndef MERGEALGO_H
#define MERGEALGO_H

// CPU merge sort of array[0 .. n) with one scratch buffer allocated up front
void mergeSort(int *array, long n);

// merges the sorted arrays a (na elements) and b (nb elements) into out
void mergeTwo(const int *a, long na, const int *b, long nb, int *out);

#endif
