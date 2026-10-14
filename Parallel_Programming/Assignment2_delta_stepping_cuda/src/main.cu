#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdlib>
#include <climits>
#include <fstream>

#include "graph.h"
#include "cudaKernel.h"

using namespace std;

// Usage: ./sssp <graph.mtx> <delta, 0 = average edge weight + 1> <source> [<source> ...]
// For every source vertex: sequential delta-stepping, Dijkstra (reference) and delta-stepping on
// the GPU are run and their distance arrays are compared.

#define CUDA_CHECK(call)                                                                  \
    {                                                                                     \
        cudaError_t e = (call);                                                           \
        if (e != cudaSuccess)                                                             \
        {                                                                                 \
            cerr << "CUDA error " << cudaGetErrorString(e) << " at line " << __LINE__ << endl; \
            exit(1);                                                                      \
        }                                                                                 \
    }

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        cout << "usage: " << argv[0] << " <graph.mtx> <delta (0 = average weight + 1)> <source> [<source> ...]" << endl;
        return 1;
    }
    string readFile = argv[1];
    int delta = atoi(argv[2]);
    vector<int> rootNodes;
    for (int i = 3; i < argc; i++)
    {
        rootNodes.push_back(atoi(argv[i]));
    }

    int noOfNodes, avgWeight;
    long noOfEdgesInFile;
    vector<vector<edge>> adjacency_list;

    cout << "Reading " << readFile << endl;
    readMTXFile(readFile, adjacency_list, avgWeight, noOfEdgesInFile, noOfNodes);
    if (delta <= 0)
    {
        delta = avgWeight;
    }

    CSRGraph g;
    adjListToCSR(adjacency_list, delta, g);
    vector<vector<edge>>().swap(adjacency_list);
    long noOfEdges = g.noOfEdges;
    cout << "nodes = " << noOfNodes << ", undirected edges = " << noOfEdgesInFile << ", stored (directed) edges = " << noOfEdges
         << ", delta = " << delta << endl;

    // ---------------- device memory, allocated and filled once for all source vertices ----------------
    auto h2dStart = chrono::high_resolution_clock::now();
    int *dist_cuda, *bucketOf_cuda, *settledIn_cuda;
    int *rowPointer_cuda, *lightEnd_cuda, *columnIndex_cuda, *weights_cuda;
    int *flag_cuda; // one int used for "changed" and for "next bucket"

    CUDA_CHECK(cudaMalloc(&dist_cuda, noOfNodes * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&bucketOf_cuda, noOfNodes * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&settledIn_cuda, noOfNodes * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&rowPointer_cuda, (noOfNodes + 1) * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&lightEnd_cuda, noOfNodes * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&columnIndex_cuda, noOfEdges * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&weights_cuda, noOfEdges * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&flag_cuda, sizeof(int)));

    CUDA_CHECK(cudaMemcpy(rowPointer_cuda, g.rowPtr.data(), (noOfNodes + 1) * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(lightEnd_cuda, g.lightEnd.data(), noOfNodes * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(columnIndex_cuda, g.colInd.data(), noOfEdges * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(weights_cuda, g.weights.data(), noOfEdges * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaDeviceSynchronize());
    auto h2dStop = chrono::high_resolution_clock::now();
    double h2dMs = chrono::duration_cast<chrono::microseconds>(h2dStop - h2dStart).count() / 1000.0;
    cout << "graph copied to the GPU in " << h2dMs << " ms (done once)" << endl << endl;

    int blockSize = 256;
    int gridSize = (noOfNodes + blockSize - 1) / blockSize;

    vector<int> distGpu(noOfNodes), distSeq, distRef;
    bool allCorrect = true;
    ofstream csv("output/timings_delta" + to_string(delta) + ".csv");
    csv << "source,reachable_nodes,max_distance,seq_ms,gpu_ms,gpu_ms_with_result_copy,speedup,buckets,light_launches,total_launches,correct" << endl;

    for (size_t rn = 0; rn < rootNodes.size(); rn++)
    {
        int root = rootNodes[rn];
        if (root < 0 || root >= noOfNodes)
        {
            cout << "source " << root << " is outside the graph, skipped" << endl;
            continue;
        }

        // ---------------- sequential delta-stepping and Dijkstra reference ----------------
        double seqMs = seqDeltaStepping(g, root, distSeq);
        dijkstra(g, root, distRef);

        // ---------------- delta-stepping on the GPU ----------------
        auto start = chrono::high_resolution_clock::now();

        distanceInitialization<<<gridSize, blockSize>>>(dist_cuda, bucketOf_cuda, settledIn_cuda, noOfNodes, root);

        long lightLaunches = 0, totalLaunches = 1, bucketsProcessed = 0;
        int buc = 0;
        int flag;
        while (true)
        {
            bucketsProcessed++;
            // light edges: repeat until no node is left in the current bucket
            do
            {
                CUDA_CHECK(cudaMemset(flag_cuda, 0, sizeof(int)));
                lightEdgesKernel<<<gridSize, blockSize>>>(dist_cuda, bucketOf_cuda, settledIn_cuda, rowPointer_cuda, lightEnd_cuda,
                                                          columnIndex_cuda, weights_cuda, noOfNodes, buc, delta, flag_cuda);
                CUDA_CHECK(cudaMemcpy(&flag, flag_cuda, sizeof(int), cudaMemcpyDeviceToHost));
                lightLaunches++;
                totalLaunches++;
            } while (flag != 0);

            // heavy edges of the nodes settled in this bucket
            heavyEdgesKernel<<<gridSize, blockSize>>>(dist_cuda, bucketOf_cuda, settledIn_cuda, rowPointer_cuda, lightEnd_cuda,
                                                      columnIndex_cuda, weights_cuda, noOfNodes, buc, delta);

            // jump directly to the next non-empty bucket
            flag = INF_DIST;
            CUDA_CHECK(cudaMemcpy(flag_cuda, &flag, sizeof(int), cudaMemcpyHostToDevice));
            nextBucketKernel<<<gridSize, blockSize>>>(bucketOf_cuda, noOfNodes, flag_cuda);
            CUDA_CHECK(cudaMemcpy(&flag, flag_cuda, sizeof(int), cudaMemcpyDeviceToHost));
            totalLaunches += 2;
            if (flag == INF_DIST)
            {
                break;
            }
            buc = flag;
        }
        CUDA_CHECK(cudaDeviceSynchronize());
        auto stop = chrono::high_resolution_clock::now();
        double gpuMs = chrono::duration_cast<chrono::microseconds>(stop - start).count() / 1000.0;

        // Copy data from device to host
        CUDA_CHECK(cudaMemcpy(distGpu.data(), dist_cuda, noOfNodes * sizeof(int), cudaMemcpyDeviceToHost));
        auto stop2 = chrono::high_resolution_clock::now();
        double gpuMsCopy = chrono::duration_cast<chrono::microseconds>(stop2 - start).count() / 1000.0;

        // ---------------- correctness ----------------
        long mismatchSeq = 0, mismatchGpu = 0, reachable = 0;
        int maxDist = 0;
        for (int i = 0; i < noOfNodes; i++)
        {
            if (distSeq[i] != distRef[i])
                mismatchSeq++;
            if (distGpu[i] != distRef[i])
            {
                if (mismatchGpu < 5)
                    cout << "      mismatch at node " << i << ": GPU " << distGpu[i] << ", Dijkstra " << distRef[i] << endl;
                mismatchGpu++;
            }
            if (distRef[i] != INT_MAX)
            {
                reachable++;
                if (distRef[i] > maxDist)
                    maxDist = distRef[i];
            }
        }
        bool ok = (mismatchSeq == 0 && mismatchGpu == 0);
        allCorrect = allCorrect && ok;

        cout << "source " << root << ": reachable nodes = " << reachable << ", largest distance = " << maxDist << endl;
        cout << "   sequential delta-stepping : " << seqMs << " ms   (nodes differing from Dijkstra: " << mismatchSeq << ")" << endl;
        cout << "   GPU delta-stepping        : " << gpuMs << " ms   (" << gpuMsCopy << " ms with the result copied back; nodes differing from Dijkstra: "
             << mismatchGpu << ")" << endl;
        cout << "   speedup = " << seqMs / gpuMs << "   buckets = " << bucketsProcessed << ", light-edge launches = " << lightLaunches
             << ", all kernel launches = " << totalLaunches << "   " << (ok ? "CORRECT" : "WRONG") << endl;
        csv << root << "," << reachable << "," << maxDist << "," << seqMs << "," << gpuMs << "," << gpuMsCopy << "," << seqMs / gpuMs << ","
            << bucketsProcessed << "," << lightLaunches << "," << totalLaunches << "," << (ok ? "yes" : "NO") << endl;

        // distances of this source, one per line (node, distance; -1 = not reachable)
        if (noOfNodes <= 100)
        {
            for (int i = 0; i < noOfNodes; i++)
                cout << "      dist[" << i << "] = " << (distGpu[i] == INT_MAX ? -1 : distGpu[i]) << endl;
        }
    }
    csv.close();

    // Free memory
    cudaFree(dist_cuda);
    cudaFree(bucketOf_cuda);
    cudaFree(settledIn_cuda);
    cudaFree(rowPointer_cuda);
    cudaFree(lightEnd_cuda);
    cudaFree(columnIndex_cuda);
    cudaFree(weights_cuda);
    cudaFree(flag_cuda);

    cout << endl << (allCorrect ? "ALL SOURCES CORRECT" : "SOME SOURCES ARE WRONG") << endl;
    return allCorrect ? 0 : 2;
}
