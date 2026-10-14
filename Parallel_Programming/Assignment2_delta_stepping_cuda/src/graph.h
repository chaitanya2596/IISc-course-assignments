#ifndef GRAPH_H
#define GRAPH_H
#include <string>
#include <vector>

struct edge
{
  int to;
  int weights;
};

// Graph in compressed sparse row (CSR) form.
// For every node the light edges (weight <= delta) are stored first and the heavy edges after them:
//   light edges of u : rowPtr[u]   .. lightEnd[u]-1
//   heavy edges of u : lightEnd[u] .. rowPtr[u+1]-1
struct CSRGraph
{
  int noOfNodes = 0;
  long noOfEdges = 0; // directed edge count (each undirected edge is stored twice)
  int delta = 1;
  std::vector<int> rowPtr, lightEnd, colInd, weights;
};

// reads a symmetric MatrixMarket file with integer weights ("row col weight" per line)
void readMTXFile(std::string inputLocation, std::vector<std::vector<edge>> &adjacency_list, int &avgWeight, long &noOfEdges, int &noOfNodes);

void adjListToCSR(std::vector<std::vector<edge>> &adjacency_list, int delta, CSRGraph &g);

// sequential delta-stepping, returns time of the algorithm in milliseconds
double seqDeltaStepping(const CSRGraph &g, int root, std::vector<int> &dist);

// Dijkstra with a binary heap, used only as an independent reference for correctness
void dijkstra(const CSRGraph &g, int root, std::vector<int> &dist);

#endif
