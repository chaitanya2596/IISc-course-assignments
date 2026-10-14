#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <ctime>

using namespace std;

void generateRandomGraph(int n)
{

    vector<vector<int>> iniAdjacency_list;
    // int max_edges = n * (n - 1) / 2;

    srand(time(0));
    // int edges = rand() % max_edges + 1;

    // Adjacency Matrix generation
    int count = 0;
    for (int i = 0; i < n - 1; i++)
    {
        for (int k = i + 1; k < n; k++)
        {
            vector<int> temp;
            if (rand() % 100 <= 50)
            {
                temp.push_back(i);
                temp.push_back(k);
                temp.push_back(rand() % 100 + 1);
                iniAdjacency_list.push_back(temp);
                count++;
            }
        }
    }

    cout << endl;
    cout << endl;
    // print adjacency matrix
    for (int i = 0; i < iniAdjacency_list.size(); i++)
    {
        for (int j = 0; j < iniAdjacency_list[i].size(); j++)
        {
            cout << iniAdjacency_list[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;
    cout << endl;

    cout << endl
         << "iniAdjacency_list.size(): " << iniAdjacency_list.size() << endl;
    cout << "count : " << count << endl;

    ofstream file;
    string fileName = "input/testCase_" + to_string(n) + ".mtx";
    file.open(fileName);
    file << "%MatrixMarket matrix coordinate pattern symmetric" << endl;

    // writing to a file
    file << n << " " << n << " " << count << endl;
    for (int i = 0; i < iniAdjacency_list.size(); i++)
    {
        file << iniAdjacency_list[i][0] << " " << iniAdjacency_list[i][1] << " " << iniAdjacency_list[i][2] << endl;
    }

    // cout << "Edges: " << count << endl;
    // cout << "Vertices: " << n << endl;
    // cout << "max_edges: " << max_edges << endl;
    file.close();
}
