#include "KMP.h"

void KMP::CalcLps(string pat, int m, int* lps)
{
	lps[0] = 0;
	int len = 0;
	int i = 1;

	while (i < m)
	{
		if (pat[i] == pat[len])
		{
			len++;
			lps[i] = len;
			i++;
		}
		else
		{
			if (len != 0)
			{
				len = lps[len - 1];
			}
			else
			{
				lps[i] = 0;
				i++;
			}
		}
	}
}

vector<int> KMP::FindString(string text, string pat)
{
	int a = pat.length();//patterns length
	int b = text.length();//string length
	int i = 0;
	int j = 0;
	int* lps = new int[a];//dynamic array implemnetation
	vector<int> output;

	CalcLps(pat, a, lps);

	while (i < b)
	{
		if (pat[j] == text[i])
		{
			j++;
			i++;
		}

		if (j == a)
		{
			output.push_back(i - j);
			j = lps[j - 1];
		}

		else if (i < b && pat[j] != text[i])
		{
			if (j != 0)
				j = lps[j - 1];
			else
				i = i + 1;
		}
	}

	delete[] lps;//free memory

	return output;
}