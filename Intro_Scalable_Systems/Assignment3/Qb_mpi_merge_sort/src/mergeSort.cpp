#include <vector>
#include <cstring>
#include <cstdlib>

#include "mergeSort.h"

using namespace std;

void mergeTwo(const int *a, int na, const int *b, int nb, int *out)
{
	int i = 0, j = 0, k = 0;
	while (i < na && j < nb)
	{
		if (a[i] <= b[j])
		{
			out[k++] = a[i++];
		}
		else
		{
			out[k++] = b[j++];
		}
	}
	// Copy the remaining elements of a[] or b[], if there are any
	if (i < na)
	{
		memcpy(out + k, a + i, (na - i) * sizeof(int));
	}
	if (j < nb)
	{
		memcpy(out + k, b + j, (nb - j) * sizeof(int));
	}
}

// sorts array[begin .. end) using tmp as scratch space
static void mergeSortRec(int *array, int *tmp, int begin, int end)
{
	int n = end - begin;
	if (n <= 1)
		return;
	if (n <= 16)
	{
		// insertion sort for very short pieces
		for (int i = begin + 1; i < end; i++)
		{
			int key = array[i];
			int j = i - 1;
			while (j >= begin && array[j] > key)
			{
				array[j + 1] = array[j];
				j--;
			}
			array[j + 1] = key;
		}
		return;
	}
	int mid = begin + n / 2;
	mergeSortRec(array, tmp, begin, mid);
	mergeSortRec(array, tmp, mid, end);
	if (array[mid - 1] <= array[mid])
		return; // the two halves are already in order
	mergeTwo(array + begin, mid - begin, array + mid, end - mid, tmp + begin);
	memcpy(array + begin, tmp + begin, n * sizeof(int));
}

void mergeSort(int *array, int n)
{
	vector<int> tmp(n);
	mergeSortRec(array, tmp.data(), 0, n);
}

static int compareInts(const void *a, const void *b)
{
	int x = *(const int *)a, y = *(const int *)b;
	return (x > y) - (x < y);
}

void quickSort(int *array, int n)
{
	qsort(array, n, sizeof(int), compareInts);
}
