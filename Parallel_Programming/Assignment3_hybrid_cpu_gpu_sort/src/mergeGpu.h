#ifndef MERGEGPU_H
#define MERGEGPU_H

// Sorts data[0 .. size) on the GPU (bottom-up merge sort). Returns the time in seconds spent on
// the GPU including the copies to and from the device. Exits with a message on a CUDA error.
double gpuMergeSort(int *data, long size);

// free and total memory of the GPU in bytes
void gpuMemory(long *freeBytes, long *totalBytes);

#endif
