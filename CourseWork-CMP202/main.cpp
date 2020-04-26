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
using std::cin;
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
//size of text 1 = 174804
int sizeofText1 = 174804;

//size of text 2 = 262204
int sizeofText2 = 262204;

//size of text 3 = 524322
int sizeofText3 = 524322;

//mutex for pattern
mutex jumble_mutex;
condition_variable jumble_cond;
int jumbleready = 0;

//number of searches in each logfile
const int numoftimes = 100;

//number of times pattern found in each logfile
int numfile1 = 0;
int numfile2 = 0;
int numfile3 = 0;

//total times pattern found
int totalfile1 = 0;
int totalfile2 = 0;
int totalfile3 = 0;

//word to be jumbled
string jumble_name = "";

//return type of the difference in start and end times is int64_t	
//holds total time taken
int64_t kmp_maxtimes1 = 0;
int64_t kmp_maxtimes2 = 0;
int64_t kmp_maxtimes3 = 0;

//holds a list of all the times
vector<int64_t> kmp_times1;
vector<int64_t> kmp_times2;
vector<int64_t> kmp_times3;

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

//last proper suffix
void CalcLps(int m, int* lps, string pat)
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

	thread Calc_thread(CalcLps, a, lps, pat);
	Calc_thread.join();
	//CalcLps(a, lps, pat);

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

//times to complete search
void Times_one(int64_t K_ms)
{
	kmp_maxtimes1 += K_ms;
	kmp_times1.push_back(K_ms);
}
void Times_two(int64_t K_ms)
{
	kmp_maxtimes2 += K_ms;
	kmp_times2.push_back(K_ms);
}
void Times_three(int64_t K_ms)
{
	kmp_maxtimes3 += K_ms;
	kmp_times3.push_back(K_ms);
}

//Time KMP search algorithm
void RunKMP(string text, string pat)
{
	//Start timing for KMP
	auto K_begin = std::chrono::steady_clock::now();

	vector<int> KMP_pos = FindString(text, pat);//Call KMP search function

	int i = 0;//number of times
	for (auto k : KMP_pos)
	{
		i++;
	}
	//cout << "KMP -  " << i << endl;

	auto K_end = std::chrono::steady_clock::now();
	auto K_ms = std::chrono::duration_cast<std::chrono::milliseconds>(K_end - K_begin).count();//compute difference
	
	if (text.size() == sizeofText1)//file one
	{
		Times_one(K_ms);
		numfile1 = i;
		totalfile1 += i;
	} 
	else if (text.size() == sizeofText2)//file two
	{		
		Times_two(K_ms);
		numfile2 = i;
		totalfile2 += i;
	}
	else if (text.size() == sizeofText3)//file three
	{	
		Times_three(K_ms);
		numfile3 = i;
		totalfile3 += i;
	}
	//end timing of KMP 
}

//set name to be jumbled
void setJumble(string name)
{
	unique_lock<mutex> locker(jumble_mutex);
	jumble_name = name;
	jumbleready = 1;
	jumble_cond.notify_one();
}

//jumble name
void JumbleName()
{
	unique_lock<mutex> locker(jumble_mutex);
	while (jumbleready != 1)
	{
		jumble_cond.wait(locker);
	}
	
	int jumble_length = jumble_name.size();
	for (int i = 0; i < jumble_name.size(); i++)
	{
		int random_pos1 = (rand() % jumble_length);
		int random_pos2 = (rand() % jumble_length);
		char temp = jumble_name[random_pos1];
		jumble_name[random_pos1] = jumble_name[random_pos2];
		jumble_name[random_pos2] = temp;
	}
	jumbleready = 0;
}

int main()
{
	srand(time(NULL));

	//number of words in each pieces of text
	int Words1 = 23352;
	int Words2 = 35028;
	int Words3 = 70056;	

	//holds the text in the logfiles
	string text1;
	string text2;
	string text3;

	//load files
	/*load_file("Logfile1.txt", text1);
	load_file("Logfile2.txt", text2);
	load_file("Logfile3.txt", text3);#*/

	thread load_file_thread1(load_file, "Logfile1.txt", std::ref(text1));
	thread load_file_thread2(load_file, "Logfile2.txt", std::ref(text2));
	thread load_file_thread3(load_file, "Logfile3.txt", std::ref(text3));
	
	//join threads
	load_file_thread1.join();
	load_file_thread2.join();
	load_file_thread3.join();

	//Threads to run KMP function for each logfile
	thread* KMP_thread1[numoftimes];
	thread* KMP_thread2[numoftimes];
	thread* KMP_thread3[numoftimes];

	//name to look for
	string pat1 = "Alby";
	string pat2 = "Jeffery";
	string pat3 = "Alexandre";

	//name to jumble
	string name = "";

	//jumble times
	//holds total times
	float jumble_maxtimes = 0;

	//holds all the times
	vector<float> jumble_times;

	//how many times each word got jumbled
	int Ajumble = 0;
	int Jjumble = 0;
	int Aljumble = 0;

	//jumble threads
	thread* setValueThread[numoftimes];
	thread* JumblevalueThread[numoftimes];

	//Times one loop
	int64_t oneRun = 0;
	vector<int64_t> oneRun_times;

	int playerChoice = 3;

	//How many threads to run
	while (playerChoice > 2)
	{
		cout << "Run Program with: \n";
		cout << "1: All threads\n";
		cout << "2: Grouped Threads\n";
		cin >> playerChoice;
	}	

	cout << "Searching Alby in file 1\n";
	cout << "Searching Jeffery in file 2\n";
	cout << "Searching Alexandre in file 3\n";
	cout << "Jumbling words\n";
	cout << "Running...\n";	

	auto Total_begin = std::chrono::steady_clock::now();

	for (int i = 0; i < numoftimes; i++)
	{
		auto OneRun_begin = std::chrono::steady_clock::now();

		if (playerChoice == 1)
		{
			//All threads		
			KMP_thread1[i] = new thread(RunKMP, text1, pat1);
			KMP_thread2[i] = new thread(RunKMP, text2, pat2);
			KMP_thread3[i] = new thread(RunKMP, text3, pat3);

			//jumble words
			auto Jumble_begin = std::chrono::steady_clock::now();

			int num = rand() % 3 + 1;

			switch (num)
			{
			case 1:
				name = "Alby";
				Ajumble++;
				break;
			case 2:
				name = "Jeffery";
				Jjumble++;
				break;
			case 3:
				name = "Alexandre";
				Aljumble++;
				break;
			default:
				break;
			}

			setValueThread[i] = new thread(setJumble, name);
			JumblevalueThread[i] = new thread(JumbleName);
			//end of jumble

			auto Jumble_end = std::chrono::steady_clock::now();
			float Jumble_ms = std::chrono::duration_cast<std::chrono::nanoseconds>(Jumble_end - Jumble_begin).count();//compute difference

			Jumble_ms = Jumble_ms / 1000000;//change to milliseconds
			jumble_maxtimes += Jumble_ms;
			jumble_times.push_back(Jumble_ms);

			//join threads
			KMP_thread1[i]->join();
			delete KMP_thread1[i];

			KMP_thread2[i]->join();
			delete KMP_thread2[i];

			KMP_thread3[i]->join();
			delete KMP_thread3[i];

			setValueThread[i]->join();
			delete setValueThread[i];

			JumblevalueThread[i]->join();
			delete JumblevalueThread[i];
		}
		else
		{
			//Grouped threads

			RunKMP(text1, pat1);
			RunKMP(text2, pat2);
			RunKMP(text3, pat3);

			auto Jumble_begin = std::chrono::steady_clock::now();
			//jumble words
			int num = rand() % 3 + 1;

			switch (num)
			{
			case 1:
				name = "Alby";
				Ajumble++;
				break;
			case 2:
				name = "Jeffery";
				Jjumble++;
				break;
			case 3:
				name = "Alexandre";
				Aljumble++;
				break;
			default:
				break;
			}

			setJumble(name);
			JumbleName();	

			auto Jumble_end = std::chrono::steady_clock::now();
			float Jumble_ms = std::chrono::duration_cast<std::chrono::nanoseconds>(Jumble_end - Jumble_begin).count();//compute difference

			Jumble_ms = Jumble_ms / 1000000;//change to milliseconds
			jumble_maxtimes += Jumble_ms;
			jumble_times.push_back(Jumble_ms);
		}

		auto OneRun_end = std::chrono::steady_clock::now();
		auto OneRun_ms = std::chrono::duration_cast<std::chrono::milliseconds>(OneRun_end - OneRun_begin).count();//compute difference
		oneRun += OneRun_ms;
		oneRun_times.push_back(OneRun_ms);
	}

	auto Total_end = std::chrono::steady_clock::now();
	auto Total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(Total_end - Total_begin).count();//compute difference

	cout << "Search Complete\n\n";
	cout << "Alby found " << numfile1 << " times in file one\n";
	cout << "Jeffery found " << numfile2 << " times in file two\n";
	cout << "Alexandre found " << numfile3 << " times in file three\n";
	cout << "Total number found in file 1: " << totalfile1 << " times\n";
	cout << "Total number found in file 2: " << totalfile2 << " times\n";
	cout << "Total number found in file 3: " << totalfile3 << " times\n";
	cout << endl;
	cout << "Alby jumbled " << Ajumble << " times\n";
	cout << "Jeffery jumbled " << Jjumble << " times\n";
	cout << "Alexandre jumbled " << Aljumble << " times\n";
	cout << endl;

	//total time taken in seconds
	auto seconds = Total_ms / 1000;
	cout << "Total time taken: " << seconds <<  " seconds" << endl;

	//compute the average time for text 1
	auto avkmp1_time = kmp_maxtimes1 / numoftimes;
	cout << endl;
	cout << "Average KMP time 1: " << avkmp1_time << " ms" << endl;

	//compute the average time for text 2
	auto avkmp2_time = kmp_maxtimes2 / numoftimes;
	cout << endl;
	cout << "Average KMP time 2: " << avkmp2_time << " ms" << endl;

	//compute the average time for text 3
	auto avkmp3_time = kmp_maxtimes3 / numoftimes;
	cout << endl;
	cout << "Average KMP time 3: " << avkmp3_time << " ms" << endl;

	//compute the average time to jumble words
	auto avjumble_time = jumble_maxtimes / numoftimes;
	cout << endl;
	cout << "Average Jumble time: " << avjumble_time <<  " ms" << endl;

	//compute the average time for one run
	auto avOneRun_time = oneRun / numoftimes;
	cout << endl;
	cout << "Average OneRun time: " << avOneRun_time << " ms" << endl;

	//write to excel
	/*int run = 0;
	ofstream AllTimes_file("AllTimes.csv");
	AllTimes_file << " " << "," <<  "Jumble times" << endl;

	while (jumble_times.size() != 0)
	{
		run++;
		AllTimes_file << run << "," << jumble_times.back() << endl;
		jumble_times.pop_back();
	}
	AllTimes_file.close();*/

	//write to excel
	/*int run2 = 0;
	ofstream OneRun_file("OneRun.csv");
	OneRun_file << " " << "," <<  "OneRun times" << endl;

	while (oneRun_times.size() != 0)
	{
		run2++;
		OneRun_file << run2 << "," << oneRun_times.back() << endl;
		oneRun_times.pop_back();
	}
	OneRun_file.close();*/

	system("pause");
}