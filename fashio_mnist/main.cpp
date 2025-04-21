#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include "wrapped_calcer.h"
using namespace std;
namespace fs = std::filesystem;

void printHelp()
{
	cout << "CatBoost prediction" << endl;
	cout << "Usage: fashio_mnist <test_data> <model>" << endl;
	cout << "Example: fashio_mnist model.cbm test_data_catboost.txt";
	cout << "Output: probability, correct_answers/total_test_data_rows";
}

bool checkFile(const std::string &f)
{
	if(fs::exists(f))
		return true;
	cout << "error: file " << f << " not exist" << endl;
	return false;
}

int main(int argc, char** argv) 
{
	if(argc < 3) { printHelp(); return 0; }

	string testDataFile;
	string modelFile;
	testDataFile = argv[1];
	modelFile = argv[2];
	if(! checkFile(testDataFile))
		return -1;
	if(! checkFile(modelFile))
		return -1;

	ModelCalcerWrapper calculator;
	calculator.init_from_file(modelFile);

	// это пришлось дописать во враппер, там была TODO на это дело
	// возможно уже есть в новой версии catBoost, но под windows собранные бинарники есть только для старой версии
	calculator.SetPredictionType(APT_PROBABILITY); 

	int totalResults = 0;
	int correctResults = 0;

	ifstream ifs(testDataFile);
	string line;
	while(getline(ifs,line))
	{
		istringstream ss{line};
		int expected;
		ss >> expected;

		vector<float> features;
		float val;
		while(ss, ss >> val)
			features.push_back(val);

		vector<double> res = calculator.CalcFlatMulti(features);
		auto maxIt = max_element(res.begin(),res.end());
		int result = static_cast<int>(distance(res.begin(),maxIt));

		//cout << expected << " -> "<< result << endl;

		totalResults++;
		if(result == expected)
			correctResults++;
	}
	cout << correctResults / (double)totalResults << endl;
	return 0;
}

