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
#include <mutex>

using std::cout;
using std::endl;
using std::ofstream;
using std::thread;
using std::cerr;
using std::string;
using std::ifstream;
using std::vector;
using std::mutex;
using std::condition_variable;
using std::unique_lock;

//global variables
//mutex to lock kmp function
mutex kmp_mutex;

//mutex for pattern
mutex pat_mutex;
condition_variable pat_cond;
int patready = 0;

//holds the difference in start and end times
int64_t K_ms = 0;

//number of searches in each logfile
const int numoftimes = 100;

//name to look for
string pat = "";

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

//last proper suffix for KMP
void CalcLps(int m, int* lps)
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
vector<int> FindString(string text)
{
	int a = pat.length();//patterns length
	int b = text.length();//string length
	int i = 0;
	int j = 0;
	int* lps = new int[a];//dynamic array implemnetation
	vector<int> output;

	thread Calc_thread(CalcLps, a, lps);
	Calc_thread.join();
	//CalcLps(a, lps);

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

//Time KMP search algorithm
void RunKMP(string text)
{
	unique_lock<mutex> locker(kmp_mutex);//so K_ms can be added to the correct times varibales
	// Start timing for KMP
	auto K_begin = std::chrono::steady_clock::now();

	vector<int> KMP_pos = FindString(text);//Call KMP search function

	int i = 0;//number of times
	for (auto k : KMP_pos)
	{
		i++;
	}
	//cout << "KMP -  " << i << endl;

	auto K_end = std::chrono::steady_clock::now();
	K_ms = std::chrono::duration_cast<std::chrono::milliseconds>(K_end - K_begin).count();//compute difference
	//cout << "KMP time: "<< K_ms << endl;
	//end timing of KMP 
}

//Change the pattern to look for
void setPat1()
{
	unique_lock<mutex> locker(pat_mutex);
	pat = "Alby";
	patready = 1;
	pat_cond.notify_one();
}
void setPat2()
{
	unique_lock<mutex> locker(pat_mutex);
	while (patready != 1)
	{
		pat_cond.wait(locker);
	}	
	pat = "Jeffery";
	patready = 2;
}
void setPat3()
{
	unique_lock<mutex> locker(pat_mutex);
	while (patready != 2)
	{
		pat_cond.wait(locker);
	}
	pat = "Alexandre";
	patready = 3;
}

int main()
{
	//number of words in each pieces of text
	int Words1 = 23352;
	int Words2 = 35028;
	int Words3 = 70056;

	//holds the text in the logfiles
	string text1;
	string text2;
	string text3;

	//return type of the difference in start and end times is int64_t	
	//holds total time taken
	int64_t kmp_maxtimes1 = 0;
	int64_t kmp_maxtimes2 = 0;
	int64_t kmp_maxtimes3 = 0;

	//holds a list of all the times
	vector<int64_t> kmp_times1;
	vector<int64_t> kmp_times2;
	vector<int64_t> kmp_times3;

	//load file one
	//load_file("Logfile1.txt", text1);
	//load_file("Logfile4.txt", text1);

	thread load_file_thread1(load_file, "Logfile1.txt", std::ref(text1));
	//thread load_file_thread1(load_file, "Logfile4.txt", std::ref(text1));
	//load_file_thread1.join();

	//load file two
	//load_file("Logfile2.txt", text2);
	//load_file("Logfile5.txt", text2);

	thread load_file_thread2(load_file, "Logfile2.txt", std::ref(text2));
	//thread load_file_thread2(load_file, "Logfile5.txt", std::ref(text2));
	//load_file_thread2.join();

	//load file three
	//load_file("Logfile3.txt", text3);
	//load_file("Logfile6.txt", text3);

	thread load_file_thread3(load_file, "Logfile3.txt", std::ref(text3));
	//thread load_file_thread3(load_file, "Logfile6.txt", std::ref(text2));
	//load_file_thread3.join();

	//Threads to run KMP function for each logfile
	thread* KMP_thread1[numoftimes];
	thread* KMP_thread2[numoftimes];
	thread* KMP_thread3[numoftimes];

	//threads to change pat for each logfile
	thread* Pat1[numoftimes];
	thread* Pat2[numoftimes];
	thread* Pat3[numoftimes];

	cout << "Running...\n";
	
	for (int i = 0; i < numoftimes; i++)
	{
		//Run text 1
		Pat1[i] = new thread(setPat1);
		Pat1[i]->join();
		delete Pat1[i];

		KMP_thread1[i] = new thread(RunKMP, text1);
		KMP_thread1[i]->join();
		delete KMP_thread1[i];

		//setPat1();
		//RunKMP(text1);

		kmp_maxtimes1 += K_ms;
		kmp_times1.push_back(K_ms);
		//cout << "KMP file 1: " << K_ms << endl;
		//end of text 1

		//Run text 2
		Pat2[i] = new thread(setPat2);
		Pat2[i]->join();
		delete Pat2[i];

		KMP_thread2[i] = new thread(RunKMP, text2);
		KMP_thread2[i]->join();
		delete KMP_thread2[i];

		//setPat2();
		//RunKMP(text2);

		kmp_maxtimes2 += K_ms;
		kmp_times2.push_back(K_ms);
		//cout << "KMP file 2: " << K_ms << endl;
		//end of text 2

		//Run text 3
		Pat3[i] = new thread(setPat3);
		Pat3[i]->join();
		delete Pat3[i];

		KMP_thread3[i] = new thread(RunKMP, text3);
		KMP_thread3[i]->join();
		delete KMP_thread3[i];

		//setPat3();
		//RunKMP(text3);

		kmp_maxtimes3 += K_ms;
		kmp_times3.push_back(K_ms);
		//cout << "KMP file 3: " << K_ms << endl;
		//end of text 3
	}

	cout << "Search Complete\n";

	//average time taken
	/*
	with threads
	KMP time 1 = 328ms
	KMP time 2 = 493ms
	KMP time 3 = 971ms
	without threads
	KMP time 1 = 329ms
	KMP time 2 = 485ms
	KMP time 3 = 970ms
	*/

	//compute the average for text 1
	auto avkmp1_time = kmp_maxtimes1 / numoftimes;
	cout << endl;
	cout << "Average KMP time 1: " << avkmp1_time << endl;

	//compute the average for text 2
	auto avkmp2_time = kmp_maxtimes2 / numoftimes;
	cout << endl;
	cout << "Average KMP time 2: " << avkmp2_time << endl;

	//compute the average for text 3
	auto avkmp3_time = kmp_maxtimes3 / numoftimes;
	cout << endl;
	cout << "Average KMP time 3: " << avkmp3_time << endl;

	//write to excel
	/*ofstream my_file("Coursework.csv");
	my_file << Words1 << "," << Words2 << "," << Words3 << endl;
	my_file << avkmp1_time << "," << avkmp2_time << "," << avkmp3_time << endl;
	my_file.close();*/

	system("pause");
}