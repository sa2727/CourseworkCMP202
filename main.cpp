#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>
#include "thread"
#include "string"

//using std::chrono::duration_cast;
//using std::chrono::milliseconds;
//using std::chrono::steady_clock;
using std::cout;
using std::endl;
using std::ofstream;
using std::thread;
using std::cerr;
using std::string;
using std::ifstream;
using std::vector;

//global variables
//holds the difference in start and end times
int64_t K_ms = 0;
int64_t BM_ms = 0;

int64_t K_ms2 = 0;
int64_t BM_ms2 = 0;

int64_t K_ms3 = 0;
int64_t BM_ms3 = 0;

//holds the text in the logfiles
string text1;
string text2;
string text3;

//number of time each Run function is looped
const int numoftimes = 100;

//unable to load file
void die(const string& msg) {
	cerr << "Error: " << msg << "\n";
#ifdef _DEBUG
	abort();
#else
	exit(1);
#endif
}

//load file
void load_file(const string& filename, string& str) {
	// To make this program less fussy about where exactly it's run
	// from relative to the file, try looking in parent directories too.
	std::string directory = "";
	for (int i = 0; i < 6; i++) {
		ifstream f(directory + filename, std::ios_base::binary);
		if (!f.good()) {
			directory = "../" + directory;
			continue;
		}

		// Seek to the end of the file to find its length.
		f.seekg(0, std::ios_base::end);
		const size_t length = f.tellg();

		// Seek back to the start of the file and read the data.
		vector<char> buf(length);
		f.seekg(0);
		f.read(buf.data(), length);
		str.assign(buf.begin(), buf.end());

		return;
	}

	//die thread
	thread die_thread(die, "Unable to find " + filename);
	die_thread.join();
	//die("Unable to find " + filename);

}

//Boyer-Moore Algorithm
vector<int> find_bm_multi(const string& text, const string& pat)
{
	int pat_len = pat.size();
	int text_len = text.size();
	vector<int> output;

	int skip[256];
	for (int i = 0; i < 256; ++i)
	{
		skip[i] = pat_len; // Not in the pattern.
		for (int i = 0; i < pat_len; ++i)
		{
			skip[int(pat[i])] = (pat_len - 1) - i;
		}
	}
	for (int i = 0; i < text_len - pat_len; ++i) {
		int s = skip[int(text[i + pat_len - 1])];
		if (s != 0)
		{
			i += s - 1; // Skip forwards.
			continue;
		}
		else
		{
			int j;

			for (j = 0; j < pat_len; j++) {
				//show_context(text, j);
				if (text[i + j] != pat[j]) {
					break; // Doesn't match here.
				}
			}
			if (j == pat_len) {
				//return i; // Matched here.
				output.push_back(i);
			}
		}
	}
	return output;
}

//last proper suffix
void CalcLps(string pat, int m, int* lps)
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

//calc KMP Search algorithm
vector<int> FindString(string text, string pat)
{
	int a = pat.length();//patterns length
	int b = text.length();//string length
	int i = 0;
	int j = 0;
	int* lps = new int[a];//dynamic array implemnetation
	vector<int> output;

	thread Calc_thread(CalcLps, pat, a, lps);
	Calc_thread.join();
	//CalcLps(pat, a, lps);

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

//run first logfile
void Run1()
{	
	string pat = "Alby";//pattern to look for
	//cout << "logfile 1 " << endl;
	//cout << "Number of times pattern was found: " << endl;

	// Start timing for KMP
	auto K_begin1 = std::chrono::steady_clock::now();
	vector<int> KMP_pos1 = FindString(text1, pat);//Call KMP function

	int i = 0;//number of times
	for (auto k : KMP_pos1)
	{
		i++;
	}
	//cout << "KMP -  " << i << endl;

	auto K_end1 = std::chrono::steady_clock::now();
	auto K_ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(K_end1 - K_begin1).count();//compute difference
	//cout << "KMP time 1: "<< K_ms1 << endl;
	//end timing of KMP 

	//Start timing for BM
	auto BM_begin1 = std::chrono::steady_clock::now();
	vector<int> BM_pos1 = find_bm_multi(text1, pat);//call bm function

	int j = 0;//number of times
	for (auto b : BM_pos1)
	{
		j++;
	}
	//out << "boyer moore -  " << j << endl;

	auto BM_end1 = std::chrono::steady_clock::now();
	auto BM_ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(K_end1 - K_begin1).count();//compute difference
	//cout << "BM time 1: " << BM_ms1 << endl;
	//end timing for BM
}

//run second logfile
void Run2()
{
	//cout << "logfile 2" << endl;
	//cout << "Number of times pattern was found: " << endl;
	string pat2 = "Jeffery";//pattern to look for

	// Start timing for KMP 
	auto K_begin2 = std::chrono::steady_clock::now();
	vector<int> KMP_pos2 = FindString(text2, pat2);//Call KMP function
	int i = 0;//number of times pattern occurs in text

	for (auto k : KMP_pos2)
	{
		i++;
	}
	//cout << "KMP - " << i << endl;

	auto K_end2 = std::chrono::steady_clock::now();
	K_ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(K_end2 - K_begin2).count();//compute difference
	//cout << K_ms2 << endl;
	//stop timing of KMP

	//start timing for BM
	auto BM_begin2 = std::chrono::steady_clock::now();
	vector<int> BM_pos2 = find_bm_multi(text2, pat2);//call bm function

	int j = 0;//number of times pattern occurs in text
	for (auto b : BM_pos2)
	{
		j++;
	}
	//cout << "boyer moore - " << j << endl;

	auto BM_end2 = std::chrono::steady_clock::now();
	BM_ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(K_end2 - K_begin2).count();//compute difference
	//cout << BM_ms2 << endl;
	//end timing of BM
}

//run thrid logfile
void Run3()
{
	//cout << "logfile 3" << endl;
	//cout << "Number of times pattern was found: " << endl;
	string pat3 = "Alexandre";//pattern to look for

	// Start timing for KMP
	auto K_begin3 = std::chrono::steady_clock::now();
	vector<int> KMP_pos3 = FindString(text3, pat3);//Call KMP function
	int i = 0;//number of times pattern occurs in text

	for (auto k : KMP_pos3)
	{
		i++;
	}
	//cout << "KMP -  " << i << endl;

	auto K_end3 = std::chrono::steady_clock::now();
	K_ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(K_end3 - K_begin3).count();//computer difference
	//cout << K_ms3 << endl;
	//stop timing of KMP

	//start timing for BM
	auto BM_begin3 = std::chrono::steady_clock::now();
	vector<int> BM_pos3 = find_bm_multi(text3, pat3);//call bm function

	int j = 0;//number of times pattern occurs in text
	for (auto b : BM_pos3)
	{
		j++;
	}
	//cout << "boyer moore - " << j << endl;

	auto BM_end3 = std::chrono::steady_clock::now();
	BM_ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(K_end3 - K_begin3).count();//compute difference
	//cout << BM_ms3 << endl;
	//end timing of BM
}

//Run KMP search algorithm
void RunKMP(string pat)
{
	// Start timing for KMP
	auto K_begin = std::chrono::steady_clock::now();

	vector<int> KMP_pos = FindString(text1, pat);//Call KMP search function

	int i = 0;//number of times
	for (auto k : KMP_pos)
	{
		i++;
	}
	//cout << "KMP -  " << i << endl;

	auto K_end = std::chrono::steady_clock::now();
	K_ms = std::chrono::duration_cast<std::chrono::milliseconds>(K_end - K_begin).count();//compute difference
	//cout << "KMP time 1: "<< K_ms << endl;
	//end timing of KMP 

}

void RunBM(string pat)
{
	//Start timing for BM
	auto BM_begin = std::chrono::steady_clock::now();
	vector<int> BM_pos = find_bm_multi(text1, pat);//call BM search function

	int j = 0;//number of times
	for (auto b : BM_pos)
	{
		j++;
	}
	//out << "boyer moore -  " << j << endl;

	auto BM_end = std::chrono::steady_clock::now();
	BM_ms = std::chrono::duration_cast<std::chrono::milliseconds>(BM_end - BM_begin).count();//compute difference
	//cout << "BM time 1: " << BM_ms << endl;
	//end timing for BM

}

int main()
{
	srand(time(NULL));
	//number of words in each pieces of text
	int Words1 = 23352;
	int Words2 = 35028;
	int Words3 = 70056;
	
	//return type of the difference in start and end times is int64_t	
	//holds total time taken
	int64_t kmp1_times = 0;
	int64_t bm1_times = 0;

	int64_t kmp2_times = 0;
	int64_t bm2_times = 0;

	int64_t kmp3_times = 0;
	int64_t bm3_times = 0;

	//name to look for
	string pat;

	//load file one
	//load_file("Logfile1.txt", text1);

	thread load_file_thread1(load_file, "Logfile1.txt", std::ref(text1));
	load_file_thread1.join();
	
	thread *KMP_thread1[numoftimes];
	thread* BM_thread1[numoftimes];

	// Start timing text 1
	for (int i = 0; i < numoftimes; i++)
	{
		pat = "alby";

		//run KMP algorithm
		KMP_thread1[i] = new thread(RunKMP, pat);
		KMP_thread1[i]->join();
		delete KMP_thread1[i];

		//RunKMP(pat);

		//run BM algorithm
		BM_thread1[i] = new thread(RunBM, pat);
		BM_thread1[i]->join();
		delete BM_thread1[i];
		
		//RunBM(pat);

		kmp1_times += K_ms;
		bm1_times += BM_ms;	
		//cout << "each time KMP: " << K_ms1 << endl;
		//cout << "each time BM: " << BM_ms1 << endl;
	}
	
	//compute the average
	auto avkmp1_time = kmp1_times / numoftimes;
	auto avbm1_time = bm1_times / numoftimes;
	
	cout << endl;
	cout << "Average KMP time 1: " << avkmp1_time << endl;
	cout << "Average BM time 1: " << avbm1_time << endl;
	//end timing of text 1

/////////////////////////////

	//load file two
	//load_file("Logfile2.txt", text2);

	thread load_file_thread2(load_file, "Logfile2.txt", std::ref(text2));
	load_file_thread2.join();

	thread* KMP_thread2[numoftimes];
	thread* BM_threads2[numoftimes];

	//start timing of text 2
	for (int i = 0; i < numoftimes; i++)
	{
		pat = "Jeffery";

		KMP_thread2[i] = new thread(RunKMP, pat);
		KMP_thread2[i]->join();
		delete KMP_thread2[i];

		//RunKMP(pat);

		BM_threads2[i] = new thread(RunBM, pat);
		BM_threads2[i]->join();
		delete BM_threads2[i];

		//RunBM(pat);

		kmp2_times += K_ms;
		bm2_times += BM_ms;
	}
	//compute the average
	auto avkmp2_time = kmp2_times / numoftimes;
	auto avbm2_time = bm2_times / numoftimes;

	cout << endl;
	cout << "Average KMP time 2: " << avkmp2_time << endl;
	cout << "Average BM time 2: " << avbm2_time << endl;
	//end timing of text 2

//////////////////////////////////

	//load file three
	//load_file("Logfile3.txt", text3);

	thread load_file_thread3(load_file, "Logfile3.txt", std::ref(text3));
	load_file_thread3.join();

	thread* KMP_thread3[numoftimes];
	thread* BM_thread3[numoftimes];

	//start timing text 3
	for (int i = 0; i < numoftimes; i++)
	{
		pat = "Alexandre";

		KMP_thread3[i] = new thread(RunKMP,pat);
		KMP_thread3[i]->join();
		delete KMP_thread3[i];

		//RunKMP(pat);

		BM_thread3[i] = new thread(RunBM, pat);
		BM_thread3[i]->join();
		delete BM_thread3[i];

		//RunBM(pat);

		kmp3_times += K_ms;
		bm3_times += BM_ms;
	}
	//compute the average
	auto avkmp3_time = kmp3_times / numoftimes;
	auto avbm3_time = bm3_times / numoftimes;

	cout << endl;
	cout << "Average KMP time 3: " << avkmp3_time << endl;
	cout << "Average BM time 3: " << avbm3_time << endl;
	//end timing text 3

/////////////////////////////////		

	//write to excel
	/*ofstream my_file("Coursework.csv");
	my_file << Words1 << "," << Words2 << "," << Words3 << endl;
	my_file << K_ms1 << "," << K_ms2 << "," << K_ms3 << endl;
	my_file << BM_ms1 << "," << BM_ms2 << "," << BM_ms3 << endl;
	my_file.close();*/
	
	system("pause");

	/*average times 
	with threads
	1. KMP and BM: 169
	2. KMP and BM: 248
	3. KMP and BM: 494

	without threads
	1. KMP and BM: 170
	2. KMP and BM: 246
	3. KMP and BM: 479
	*/
}