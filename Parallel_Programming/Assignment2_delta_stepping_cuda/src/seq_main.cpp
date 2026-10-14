#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <climits>

#include "graph.h"

using namespace std;

// CPU-only driver: ./sssp_seq <graph.mtx> <delta, 0 = average edge weight + 1> <source> [<source> ...]
// Runs the sequential delta-stepping and checks it against Dijkstra. Needs no GPU.
int main(int argc, char **argv)
{
    if (argc < 4)
    {
        cout << "usage: " << argv[0] << " <graph.mtx> <delta (0 = average weight + 1)> <source> [<source> ...]" << endl;
        return 1;
    }
    int noOfNodes, avgWeight, delta = atoi(argv[2]);
    long noOfEdgesInFile;
    vector<vector<edge>> adjacency_list;
    readMTXFile(argv[1], adjacency_list, avgWeight, noOfEdgesInFile, noOfNodes);
    if (delta <= 0)
        delta = avgWeight;
    CSRGraph g;
    adjListToCSR(adjacency_list, delta, g);
    cout << "nodes = " << noOfNodes << ", undirected edges = " << noOfEdgesInFile << ", delta = " << delta << endl;

    bool allCorrect = true;
    vector<int> distSeq, distRef;
    for (int a = 3; a < argc; a++)
    {
        int root = atoi(argv[a]);
        double ms = seqDeltaStepping(g, root, distSeq);
        dijkstra(g, root, distRef);
        long mismatch = 0, reachable = 0;
        for (int i = 0; i < noOfNodes; i++)
        {
            if (distSeq[i] != distRef[i])
                mismatch++;
            if (distRef[i] != INT_MAX)
                reachable++;
        }
        allCorrect = allCorrect && mismatch == 0;
        cout << "source " << root << ": " << ms << " ms, reachable nodes = " << reachable << ", nodes differing from Dijkstra = " << mismatch << endl;
        if (noOfNodes <= 100)
            for (int i = 0; i < noOfNodes; i++)
                cout << "      dist[" << i << "] = " << (distSeq[i] == INT_MAX ? -1 : distSeq[i]) << endl;
    }
    cout << (allCorrect ? "ALL SOURCES CORRECT" : "SOME SOURCES ARE WRONG") << endl;
    return allCorrect ? 0 : 2;
}
