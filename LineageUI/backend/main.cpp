// Copyright (c) Microsoft Research 2016
// License: MIT. See LICENSE
/*
 * main.cpp
 *
 *  Created on: 14 Feb 2014
 *      Author: np183
 */


#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <iterator>
#include <numeric>
#include <math.h>
#include <algorithm>
#include <tuple>
#include "Simulation.h"
#include "LineageLib.h"

using std::cout;
using std::cerr;
using std::endl;
using std::cin;
using std::endl;
using std::string;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::stringstream;
using std::vector;
using std::map;
using std::pair;

enum class Options { load, simulation, overlap, overlapraw, existence, existencefile, exportSim, abnormalSim };

const string INITIALPROG{"P0"};
const string INITIALCOND{"SPERM_LEFT"};
const string INITIALPROGWITHCOND{INITIALPROG+"["+INITIALCOND+"]"};

void bad();
void printOptions();
void clearCinToEndOfLine();
void outputFileChooser(ofstream& of);
string ChooseConditionFromProgram(Simulation*& s, const string& name);
void readSimulation(Simulation*& s);
void timeOverlap(Simulation* s, bool rawData);
void abnormalSim(Simulation* s);
void cellCount(Simulation* s, ostream& out);
void exportSimulations(Simulation *s);


int main() {

	vector<string> programs{};
	string condition;
	simulate(programs, condition);
	checkTimeOverlap(programs, condition, condition, condition, 1);
	cellExistence(programs, condition, 1);
	simulateAbnormal(programs, condition, 1);

	Simulation* s{nullptr};
	while (true) {
		try {
			int which(0);
			printOptions();
			if (!(cin >> which)) {
				cin.clear();
				clearCinToEndOfLine();
				const string err{"Something went wrong with reading your input. Please try again."};
				throw err;
			}

			clearCinToEndOfLine();

			Options op{static_cast<Options>(which)};
			switch (op) {
			case Options::load:
				readSimulation(s);

				cout << "Would you like to see a simulation?" << endl;
				char answer;
				cin >> answer;
				/* break only on no */
				if (answer != 'y') {
					break;
				}
			case Options::simulation:
				if (!s) {
					cout << "Please upload a program first." << endl;
					break;
				}
				{
					string condition{ ChooseConditionFromProgram(s, INITIALPROG) };
					s->run(INITIALPROG, condition, -1.0, -1.0);
					cout << *s;
				}
				break;
			case Options::overlapraw:
				timeOverlap(s, true);
				break;
			case Options::overlap:
				timeOverlap(s, false);
				break;
			case Options::existencefile:
			  {
			  ofstream ofile{};
			  outputFileChooser(ofile);
				cellCount(s, ofile);
			  }
				break;
			case Options::existence:
				cellCount(s,cout);
				break;
			case Options::exportSim:
				exportSimulations(s);
				break;
			case Options::abnormalSim:
				abnormalSim(s);
				break;
			default:
				bad();
			}
		}
		catch (const string& err) {
			cerr << "Error: " << err << endl;
		}
	}
	if (s) {
		delete s;
		s=nullptr;
	}
}

void bad() {
	cout << "Bad option. Please try again." << endl;
}

void printOptions() {
	cout << "Please choose:" << endl;
	cout << "(" << static_cast<int>(Options::load) << ") Load a new program file." << endl;
	cout << "(" << static_cast<int>(Options::simulation) << ") Run a simulation." << endl;
	cout << "(" << static_cast<int>(Options::overlap) << ") Check for the time overlap of two cells." << endl;
	cout << "(" << static_cast<int>(Options::overlapraw) << ") Check for the time overlap of two cells (with raw data)." << endl;
	cout << "(" << static_cast<int>(Options::existence) << ") Check cell existence." << endl;
	cout << "(" << static_cast<int>(Options::existencefile) << ") Export cell existence." << endl;
	cout << "(" << static_cast<int>(Options::exportSim) << ") Export simulations to file." << endl;
	cout << "(" << static_cast<int>(Options::abnormalSim) << ") Search for abnormal simulation." << endl;
}

void clearCinToEndOfLine() {
	string str;
	getline(cin, str);
}

void outputFileChooser(ofstream& of) {
	cout << "Please enter the name of a file to write to." << endl;
	string outFile;
	getline(cin, outFile);
	if (!cin) {
		cin.clear();
		const string err{ "Bad input." };
		throw err;
	}
	of.open(outFile);
	if (!of) {
		const string err{ "Could not open " + outFile + " for writing." };
		throw err;
	}
}

string ChooseConditionFromProgram(Simulation*& s, const string& name) {
	cout << "Which condition would you like to start from:" << endl;
	CellProgram* cProg{ s->program(name) };
	if (!cProg) {
		const string err{ "Could not find the initial program " + name };
		throw err;
	}

	{
		CellProgram::iterator it{ cProg->begin() };
		unsigned int i{ 1 };
		while (it != cProg->end()) {
			cout << i++ << ") " << *it << endl;
			++it;
		}
	}

	unsigned int which{ 0 };
	if (!(cin >> which)) {
		const string err{ "Wrong option." };
		cin.clear();
		clearCinToEndOfLine();
		throw err;
	}

	clearCinToEndOfLine();

	CellProgram::iterator it{ cProg->begin() };
	unsigned int i{ 1 };
	while (i != which && it != cProg->end()) {
		++it;
		++i;
	}
	if (it == cProg->end()) {
		const string err{ "Wrong option." };
		throw err;
	}
	stringstream temp;
	temp << *it;
	return temp.str();
}

void readSimulation(Simulation*& s) {
	cout << "Please enter the name of a program file:" << endl;
	string file;
	getline(cin, file);
	if (s) {
		delete s;
		s = nullptr;
	}
	s = new Simulation(file);
}

void timeOverlap(Simulation* s, bool rawData) {
	if (!s) {
		cout << "Please upload a program first." << endl;
		return;
	}
	string name1, name2;
	do {
		cout << "Please enter the name of the first cell:" << endl;
		cin >> name1;
	} while (!s->program(name1));

	do {
		cout << "Please enter the name of the second cell:" << endl;
		cin >> name2;
	} while (!s->program(name2));

	unsigned int repetitions{ 0 };
	cout << "How many simulations would you like to run?" << endl;
	cin >> repetitions;

	clearCinToEndOfLine();

	vector<float> results1; // cell1 born before cell2
	vector<float> results2; // cell2 born before cell1
	for (unsigned int i{ 0 }; i<repetitions; ++i) {
		s->clear();
		s->run(INITIALPROG, INITIALCOND, -1.0, -1.0);
		pair<float, bool> p(s->overlap(name1, name2));
		if (p.second)
			results1.push_back(p.first);
		else
			results2.push_back(p.first);
	}

	auto statistics = [&](vector<float>& vec) -> vector<float> {
		std::sort(vec.begin(), vec.end());
		float min(vec[0]);
		float max(vec[vec.size() - 1]);
		const float ZERO{ 0.0 };
		float mean(std::accumulate(vec.begin(), vec.end(), ZERO) / vec.size());
		float accum = 0.0;
		std::for_each(vec.begin(), vec.end(), [&](const float f) {
			accum += (f - mean) * (f - mean);
		});
		float stdev = std::sqrt(accum / vec.size());
		float q1(vec.size() % 4 ? vec[vec.size() / 4] : (vec[vec.size() / 4 - 1] + vec[vec.size() / 4]) / 2);
		float median(vec.size() % 2 ? vec[vec.size() / 2] : (vec[vec.size() / 2 - 1] + vec[vec.size() / 2]) / 2);
		float q3(vec.size() % 4 ? vec[3 * vec.size() / 4] : (vec[3 * vec.size() / 4 - 1] + vec[3 * vec.size() / 4]) / 2);
		return vector<float> {min, max, mean, stdev, q1, median, q3};
	};

	vector<string> names{ "q1", "min", "median", "max", "q3", "mean+std", "mean-std", "mean", "stdev" };
	auto print_entry = [&](const string& name, const float& val) {
		cout << "| " << std::setw(8) << std::left << name << " | " << std::setw(6) << std::fixed << std::setprecision(2) << std::right << val << " |" << endl;
	};
	auto print_sep = [](unsigned int len) {
		std::cout.fill('-');
		std::cout.width(len);
		std::cout << "" << endl;
		std::cout.fill(' ');
	};
	auto print_all = [&](vector<float>& vec) {
		if (rawData) {
			std::ostream_iterator<float> out_it(std::cout, ", ");
			std::copy(vec.begin(), vec.end(), out_it);
			cout << endl;
		}

		vector<float> stat{ statistics(vec) };

		print_sep(8 + 7 + 6);

		std::streamsize oldprecision(cout.precision());
		cout.precision(5);
		vector<float> vals{ stat[4], stat[0], stat[5], stat[1], stat[6], stat[2] + stat[3], stat[2] - stat[3], stat[2], stat[3] };
		for (unsigned int i = 0; i<names.size(); ++i) {
			print_entry(names[i], vals[i]);
		}
		cout.precision(oldprecision);
		print_sep(8 + 7 + 6);
	};

	if (results1.size()) {
		cout << name1 << " born before " << name2 << ":" << endl;
		print_all(results1);
	}

	if (results2.size()) {
		cout << name2 << " born before " << name1 << ":" << endl;
		print_all(results2);
	}
}

void abnormalSim(Simulation* s) {
	if (!s) {
		cout << "Please upload a program first." << endl;
		return;
	}

	unsigned int repetitions{ 0 };
	cout << "Please give an upper bound for number of simulations." << endl;
	cin >> repetitions;

	clearCinToEndOfLine();

	ofstream ofile{};
	outputFileChooser(ofile);

	string condition{ ChooseConditionFromProgram(s, INITIALPROG) };

	vector<string> progs{ s->programs() };

	unsigned int numSim{ 0 };
	for (unsigned int i{ 0 }; i<repetitions; ++i) {
		s->clear();
		s->run(INITIALPROG, condition, -1.0, -1.0);
		map<string, unsigned int> res{ s->cellCount() };
		for (auto name : progs) {
			if (res.find(name) == res.end()) {
				cout << "In simulation " << numSim+1 << " (" << i << ") " << name << " was not created." << endl;
				ofile << s->toString(++numSim);
				break;
			}
			if (res.find(name)->second > 1) {
				cout << "In simulation " << numSim + 1 << " (" << i << ") " << name << " was created more than once." << endl;
				ofile << s->toString(++numSim);
				break;
			}
		}
	}
	if (0==numSim) {
		cout << "Could not find an abnormal simulation." << endl;
	}
}

void cellCount(Simulation* s, ostream& out) {
	if (!s) {
		cout << "Please upload a program first." << endl;
		return;
	}

	unsigned int repetitions{ 0 };
	cout << "How many simulations would you like to run?" << endl;
	cin >> repetitions;

	clearCinToEndOfLine();

	string condition{ ChooseConditionFromProgram(s, INITIALPROG) };

	unsigned int maxLen{ 0 };
	map<string, unsigned int> total{};
	for (unsigned int i{ 0 }; i<repetitions; ++i) {
		s->clear();
		s->run(INITIALPROG, condition, -1.0, -1.0);
		map<string, unsigned int> res{ s->cellCount() };
		for (auto nameCount : res) {
			if (nameCount.first.size() > maxLen) {
				maxLen = nameCount.first.size();
			}

			if (total.find(nameCount.first) == total.end()) {
				total.insert(nameCount);
			}
			else {
				total[nameCount.first] += nameCount.second;
			}
		}
	}

	//	out << std::setw(2+maxLen+2+5+1) << std::setfill('-') << "" << std::setfill(' ') << endl;
	for (auto nameCount : total) {
		// Do something with padding to make sure that this
		// is printed to the right length;
		//		out << "| " << std::setw(maxLen) << std::left << nameCount.first;
		//		out << " |" << std::setw(5) << std::right << nameCount.second << "|" << endl;
		//		out << std::setw(2+maxLen+2+5+1) << std::setfill('-') << "" << std::setfill(' ') << endl;
		out << nameCount.first << "," << nameCount.second << endl;
	}
}

void exportSimulations(Simulation *s) {
	if (!s) {
		cout << "Please upload a program first." << endl;
		return;
	}

	unsigned int repetitions{ 0 };
	cout << "How many simulations would you like to run?" << endl;
	if (!(cin >> repetitions)) {
		cin.clear();
		clearCinToEndOfLine();
		const string err{ "Bad input." };
		throw err;
	}

	clearCinToEndOfLine();
	string condition{ ChooseConditionFromProgram(s, INITIALPROG) };

	ofstream ofile{};
	outputFileChooser(ofile);
	for (unsigned int i{ 0 }; i<repetitions; ++i) {
		s->clear();
		s->run(INITIALPROG, condition, -1.0, -1.0);
		ofile << s->toString(i + 1);
	}
}

