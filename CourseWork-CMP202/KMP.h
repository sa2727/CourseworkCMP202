#pragma once
#include <string>
#include <vector>
using namespace std;

class KMP
{
public:
	//functions
	void CalcLps(string pat, int m, int* lps);//last proper suffix
	vector<int> FindString(string text, string pat);

};