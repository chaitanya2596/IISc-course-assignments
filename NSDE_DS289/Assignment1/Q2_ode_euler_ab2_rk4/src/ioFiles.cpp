#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <iomanip>

using namespace std;
typedef double real;



// read the input variables from the input.txt file
vector<real> readInput()
{
	ifstream importFile("input/input.txt");
	string S;
	vector<real> vector1D;
	while (getline(importFile, S))
	{
		if (S.size() > 0)
		{
			if (!(S[0] == '#'))
			{
				vector1D.push_back(stod(S));
			}
		}
	}
	importFile.close();

	return vector1D;
}


// write the function approximated values to output.csv file
void outputToCsv(int noOfSteps, vector<real> dependentVar, vector<real> independentVar,vector<real> analytical, string S)
{
	ofstream output;
	output.open("output/"+ S);
	output << setprecision(15);
	output << "dependentVar "  << "," << "independentVar " << "," << "analytical "<< endl;

	for (size_t i = 0; i <= noOfSteps; i++)
	{
		output << dependentVar[i]  << "," << independentVar[i] << "," << analytical[i] << endl;
	}
	output.close();
}
