#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <chrono>

#include "graph.h"

using namespace std;

// Sequential delta-stepping.
// Nodes are kept in buckets of width delta according to their tentative distance. For the
// current bucket the light edges are relaxed repeatedly until the bucket stays empty, then the
// heavy edges of every node that was taken out of the bucket are relaxed once.
double seqDeltaStepping(const CSRGraph &g, int root, vector<int> &dist)
{
    int delta = g.delta;
    dist.assign(g.noOfNodes, INT_MAX);
    vector<vector<int>> bucketList(1);

    dist[root] = 0;
    bucketList[0].push_back(root);

    auto start = chrono::high_resolution_clock::now();

    vector<int> settled;      // nodes removed from the current bucket
    vector<int> lastBucket(g.noOfNodes, -1); // last bucket in which a node was put into "settled"

    for (size_t buc = 0; buc < bucketList.size(); buc++)
    {
        settled.clear();
        // light edges. Entries appended while the bucket is processed are picked up by the same loop.
        for (size_t pos = 0; pos < bucketList[buc].size(); pos++)
        {
            int u = bucketList[buc][pos];
            if (dist[u] / delta != (int)buc)
            {
                continue; // stale entry: the node has moved to an earlier bucket since it was inserted
            }
            if (lastBucket[u] != (int)buc)
            {
                lastBucket[u] = buc;
                settled.push_back(u);
            }
            for (int e = g.rowPtr[u]; e < g.lightEnd[u]; e++)
            {
                int currentTo = g.colInd[e];
                int nd = dist[u] + g.weights[e];
                if (dist[currentTo] > nd)
                {
                    dist[currentTo] = nd;
                    size_t b = nd / delta;
                    if (b >= bucketList.size())
                    {
                        bucketList.resize(b + 1);
                    }
                    bucketList[b].push_back(currentTo);
                }
            }
        }
        vector<int>().swap(bucketList[buc]); // release the memory of the finished bucket

        // heavy edges of the nodes settled in this bucket
        for (size_t i = 0; i < settled.size(); i++)
        {
            int u = settled[i];
            for (int e = g.lightEnd[u]; e < g.rowPtr[u + 1]; e++)
            {
                int currentTo = g.colInd[e];
                int nd = dist[u] + g.weights[e];
                if (dist[currentTo] > nd)
                {
                    dist[currentTo] = nd;
                    size_t b = nd / delta;
                    if (b >= bucketList.size())
                    {
                        bucketList.resize(b + 1);
                    }
                    bucketList[b].push_back(currentTo);
                }
            }
        }
    }

    auto stop = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::microseconds>(stop - start).count() / 1000.0;
}

void dijkstra(const CSRGraph &g, int root, vector<int> &dist)
{
    dist.assign(g.noOfNodes, INT_MAX);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    dist[root] = 0;
    pq.push({0, root});
    while (!pq.empty())
    {
        pair<int, int> top = pq.top();
        pq.pop();
        int u = top.second;
        if (top.first != dist[u])
        {
            continue;
        }
        for (int e = g.rowPtr[u]; e < g.rowPtr[u + 1]; e++)
        {
            int v = g.colInd[e];
            int nd = dist[u] + g.weights[e];
            if (nd < dist[v])
            {
                dist[v] = nd;
                pq.push({nd, v});
            }
        }
    }
}
