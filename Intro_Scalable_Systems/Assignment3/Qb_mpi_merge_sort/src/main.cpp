#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <mpi.h>

#include "mergeSort.h"

using namespace std;

// Parallel merge sort of 1 lakh integers with MPI.
//   version A (tree)   : local sort, then processes merge in pairs, the survivors merge in pairs
//                        again, ... until the whole sorted array is in process 0
//   version B (gather) : local sort, then every process sends its piece to process 0, which merges
// Baseline: sequential quick sort on process 0. Every result is compared with the baseline.
//
// What is timed (MPI_Wtime on process 0, after a barrier): from the moment the unsorted array is
// in the memory of process 0 until the sorted array is in the memory of process 0, i.e.
// distribution + local sort + all merging and communication. Reading the text file is not timed
// in any of the three versions. As asked, every experiment is run 5 times and averaged.

// number of elements owned by each process and where they start
void distribution(int n, int size, vector<int> &counts, vector<int> &displs)
{
	counts.resize(size);
	displs.resize(size);
	int base = n / size, extra = n % size, offset = 0;
	for (int p = 0; p < size; p++)
	{
		counts[p] = base + (p < extra ? 1 : 0);
		displs[p] = offset;
		offset += counts[p];
	}
}

// version A. Returns the elapsed time on process 0; "sorted" is filled on process 0 only.
double treeMergeSort(const vector<int> &arr, int n, int rank, int size, vector<int> &sorted)
{
	vector<int> counts, displs;
	distribution(n, size, counts, displs);

	// a process that survives to the end of the tree holds everything from its own start up to
	// the end of the array at most, so that is the largest buffer it can ever need
	int capacity = n - displs[rank];
	vector<int> local(capacity), recvbuf(capacity), merged(capacity);

	MPI_Barrier(MPI_COMM_WORLD);
	double start = MPI_Wtime();

	MPI_Scatterv(arr.data(), counts.data(), displs.data(), MPI_INT, local.data(), counts[rank], MPI_INT, 0, MPI_COMM_WORLD);

	int have = counts[rank];
	mergeSort(local.data(), have);

	// merge tree: in round "step" the processes whose rank is a multiple of 2*step receive from
	// rank + step, every other process still alive sends its data to rank - step and is finished
	for (int step = 1; step < size; step *= 2)
	{
		if (rank % (2 * step) == 0)
		{
			int partner = rank + step;
			if (partner < size)
			{
				MPI_Status status;
				int recv_size;
				MPI_Recv(recvbuf.data(), capacity, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);
				MPI_Get_count(&status, MPI_INT, &recv_size);
				mergeTwo(local.data(), have, recvbuf.data(), recv_size, merged.data());
				local.swap(merged);
				have += recv_size;
			}
		}
		else
		{
			MPI_Send(local.data(), have, MPI_INT, rank - step, 0, MPI_COMM_WORLD);
			break;
		}
	}

	double elapsed = MPI_Wtime() - start;
	if (rank == 0)
	{
		local.resize(have);
		sorted.swap(local);
	}
	return elapsed;
}

// version B. Process 0 gathers all sorted pieces and merges them itself.
double gatherMergeSort(const vector<int> &arr, int n, int rank, int size, vector<int> &sorted)
{
	vector<int> counts, displs;
	distribution(n, size, counts, displs);
	vector<int> local(counts[rank]);
	vector<int> all, tmp;
	if (rank == 0)
	{
		all.resize(n);
		tmp.resize(n);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	double start = MPI_Wtime();

	MPI_Scatterv(arr.data(), counts.data(), displs.data(), MPI_INT, local.data(), counts[rank], MPI_INT, 0, MPI_COMM_WORLD);
	mergeSort(local.data(), counts[rank]);
	MPI_Gatherv(local.data(), counts[rank], MPI_INT, all.data(), counts.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		// neighbouring sorted pieces are merged two at a time until one piece is left
		vector<int> starts(displs);
		starts.push_back(n);
		while (starts.size() > 2)
		{
			vector<int> newStarts;
			for (size_t p = 0; p + 1 < starts.size(); p += 2)
			{
				newStarts.push_back(starts[p]);
				if (p + 2 < starts.size())
				{
					mergeTwo(all.data() + starts[p], starts[p + 1] - starts[p], all.data() + starts[p + 1], starts[p + 2] - starts[p + 1], tmp.data() + starts[p]);
				}
				else
				{
					// odd piece at the end, copied as it is
					for (int i = starts[p]; i < starts[p + 1]; i++)
						tmp[i] = all[i];
				}
			}
			newStarts.push_back(n);
			all.swap(tmp);
			starts.swap(newStarts);
		}
	}

	double elapsed = MPI_Wtime() - start;
	if (rank == 0)
	{
		sorted.swap(all);
	}
	return elapsed;
}

int main(int argc, char *argv[])
{
	int rank, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int time_avg = 5;
	int arr_size = 0;
	vector<int> arr;

	if (rank == 0)
	{
		// Process 0 reads the elements from the input file
		ifstream input_file("input/input.txt");
		string temp;
		while (getline(input_file, temp, ','))
		{
			arr.push_back(stoi(temp));
		}
		arr_size = arr.size();
	}
	MPI_Bcast(&arr_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// sequential quick sort (process 0 only), averaged over time_avg runs
	vector<int> serialSorted;
	double serialTime = 0;
	if (rank == 0)
	{
		for (int r = 0; r < time_avg; r++)
		{
			serialSorted = arr;
			double start = MPI_Wtime();
			quickSort(serialSorted.data(), arr_size);
			serialTime += MPI_Wtime() - start;
		}
		serialTime /= time_avg;
	}

	// one untimed warm-up of each version (first use of the MPI connections), then the timed runs
	vector<int> treeSorted, gatherSorted;
	treeMergeSort(arr, arr_size, rank, size, treeSorted);
	gatherMergeSort(arr, arr_size, rank, size, gatherSorted);

	double treeTime = 0, gatherTime = 0;
	bool correct = true;
	for (int r = 0; r < time_avg; r++)
	{
		treeTime += treeMergeSort(arr, arr_size, rank, size, treeSorted);
		gatherTime += gatherMergeSort(arr, arr_size, rank, size, gatherSorted);
		if (rank == 0)
		{
			correct = correct && (treeSorted == serialSorted) && (gatherSorted == serialSorted);
		}
	}
	treeTime /= time_avg;
	gatherTime /= time_avg;

	if (rank == 0)
	{
		cout << "processes = " << size << ", elements = " << arr_size << ", runs averaged = " << time_avg << endl;
		cout << "sequential quick sort           : " << serialTime * 1e3 << " ms" << endl;
		cout << "parallel merge sort (tree)      : " << treeTime * 1e3 << " ms   speedup " << serialTime / treeTime << endl;
		cout << "parallel merge sort (process 0) : " << gatherTime * 1e3 << " ms   speedup " << serialTime / gatherTime << endl;
		cout << (correct ? "Correctness of Program is verified!!" : "Correctness of program is NOT verified") << endl;

		ofstream outfile_speedup("output/speedup.csv", std::ios::app);
		outfile_speedup << size << "," << arr_size << "," << serialTime * 1e3 << "," << treeTime * 1e3 << "," << serialTime / treeTime << ","
						<< gatherTime * 1e3 << "," << serialTime / gatherTime << "," << (correct ? "yes" : "NO") << endl;
		outfile_speedup.close();

		ofstream output_file("output/parallel_sorted_array.txt");
		for (int i = 0; i < arr_size; i++)
		{
			output_file << treeSorted[i] << ",";
		}
		output_file.close();
	}

	MPI_Finalize();
	return 0;
}
