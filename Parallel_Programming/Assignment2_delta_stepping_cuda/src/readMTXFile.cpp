#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "graph.h"

using namespace std;

void readMTXFile(string inputLocation, vector<vector<edge>> &adjacency_list, int &avgWeight, long &noOfEdges, int &noOfNodes)
{
  ifstream file(inputLocation);
  if (!file)
  {
    cerr << "cannot open " << inputLocation << endl;
    exit(1);
  }
  string line;

  // skip the comment lines (they start with %), the first other line is "rows cols edges"
  while (getline(file, line))
  {
    if (line.size() > 0 && line[0] != '%' && line[0] != '#')
    {
      break;
    }
  }
  long rows, cols;
  stringstream header(line);
  header >> rows >> cols >> noOfEdges;

  // the node ids in the file go up to "rows" (1-based numbering), so rows + 1 entries are
  // allocated; node 0 simply has no edges in a 1-based file.
  noOfNodes = rows + 1;
  adjacency_list.assign(noOfNodes, vector<edge>());

  long weightSum = 0;
  long row, col, weight;
  long read = 0;
  while (read < noOfEdges && (file >> row >> col >> weight))
  {
    if (row < 0 || col < 0 || row >= noOfNodes || col >= noOfNodes)
    {
      cerr << "edge " << row << " " << col << " is outside the node range" << endl;
      exit(1);
    }
    adjacency_list[row].push_back({(int)col, (int)weight});
    adjacency_list[col].push_back({(int)row, (int)weight});
    weightSum = weightSum + weight;
    read++;
  }
  file.close();
  if (read != noOfEdges)
  {
    cerr << "expected " << noOfEdges << " edges but read " << read << endl;
    exit(1);
  }

  avgWeight = (weightSum / noOfEdges) + 1;
}
