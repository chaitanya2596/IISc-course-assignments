#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <iomanip>

using namespace std;
typedef double real;

// read the input variables from the input.txt file
vector<real> readInput(string fileName)
{
	ifstream importFile(fileName);
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
void outputToCsv(int noOfSteps, vector<real> t, vector<real> x, vector<real> v,string S)
{
	ofstream output;
	output.open("output/springMassDamper_rk4_"+ S + ".csv");
	output << setprecision(15);
	output << "t "	<< ","	<< "x "	<< ","	<< "v " << endl;

	for (size_t i = 0; i <= noOfSteps; i++)
	{
		output << t[i] << "," << x[i] << "," << v[i] << endl;
	}
	output.close();
}
