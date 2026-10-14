#include <iostream>
#include <vector>

#include "graph.h"

using namespace std;

// Converts the adjacency list to CSR. Inside each node the light edges (weight <= delta) are
// placed before the heavy ones, so the kernels can walk only the part they need without
// testing the weight of every edge.
void adjListToCSR(vector<vector<edge>> &adjacency_list, int delta, CSRGraph &g)
{
    int noOfNodes = adjacency_list.size();
    long total = 0;
    for (int i = 0; i < noOfNodes; i++)
    {
        total += adjacency_list[i].size();
    }

    g.noOfNodes = noOfNodes;
    g.noOfEdges = total;
    g.delta = delta;
    g.rowPtr.assign(noOfNodes + 1, 0);
    g.lightEnd.assign(noOfNodes, 0);
    g.colInd.resize(total);
    g.weights.resize(total);

    int edges = 0;
    for (int i = 0; i < noOfNodes; i++)
    {
        g.rowPtr[i] = edges;
        for (int j = 0; j < adjacency_list[i].size(); j++)
        {
            if (adjacency_list[i][j].weights <= delta)
            {
                g.colInd[edges] = adjacency_list[i][j].to;
                g.weights[edges] = adjacency_list[i][j].weights;
                edges++;
            }
        }
        g.lightEnd[i] = edges;
        for (int j = 0; j < adjacency_list[i].size(); j++)
        {
            if (adjacency_list[i][j].weights > delta)
            {
                g.colInd[edges] = adjacency_list[i][j].to;
                g.weights[edges] = adjacency_list[i][j].weights;
                edges++;
            }
        }
    }
    g.rowPtr[noOfNodes] = edges;
}
