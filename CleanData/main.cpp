#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <utility>
#include <algorithm>

const std::string delimiter = ",";

std::string toUpper(const std::string& s) {
	std::string ret = "";
	for (auto& c : s) {
		if (c >= 'a' && c <= 'z') {
			ret += c - 'a' + 'A';
		}
		else {
			ret += c;
		}
	}
	return ret;
}

int compare(const std::string& s1, const std::string& s2) {
	int l1 = s1.size();
	int l2 = s2.size();

	std::vector<std::vector<int> > dp(l1 + 1, std::vector<int>((l2 + 1), 0));

	for (int i = 1; i <= l1; i++) {
		dp[i][0] = i;
	}

	for (int j = 1; j <= l2; j++) {
		dp[0][j] = j;
	}

	for (int i = 1; i <= l1; i++) {
		for (int j = 1; j <= l2; j++) {

			if (s1[i - 1] == s2[j - 1]) {
				dp[i][j] = dp[i - 1][j - 1];
			}
			else {
				dp[i][j] = 1 + std::min(dp[i - 1][j - 1], std::min( dp[i][j - 1], dp[i - 1][j]));	
			}
		}
	}

	return dp[l1][l2];
}

int main() {

	std::string line;
	std::string token;
	std::vector<std::vector<std::string> > rows1;
	std::vector<std::vector<std::string> > rows2;

	std::ifstream file1;
	file1.open("../example/results.csv");

	if (file1.is_open()) {
		std::cout << "file1 is open!\n";
	}
	else {
		std::cout << "file1 is not open!\n";
	}

	while (std::getline(file1, line, '\n')) {
		std::vector<std::string> tokens;
		size_t pos = 0;
		while ((pos = line.find(delimiter)) != std::string::npos) {
			token = line.substr(0, pos);
			tokens.push_back(token);
			//std::cout << token << std::endl;
			line.erase(0, pos + delimiter.length());
		}
		tokens.push_back(line);
		rows1.push_back(tokens);
	}

	file1.close();

	std::ifstream file2;
	file2.open("../Constituency Summaries - Performance.csv");


	if (file2.is_open()) {
		std::cout << "file2 is open!\n";
	}
	else {
		std::cout << "file2 is not open!\n";
	}

	while (std::getline(file2, line, '\n')) {
		std::vector<std::string> tokens;
		size_t pos = 0;
		while ((pos = line.find(delimiter)) != std::string::npos) {
			token = line.substr(0, pos);
			tokens.push_back(token);
			//std::cout << token << std::endl;
			line.erase(0, pos + delimiter.length());
		}
		tokens.push_back(line);
		rows2.push_back(tokens);
	}

	file2.close();

	std::cout << "Rows read " << rows1.size() << "\n";
	std::cout << "Columns[1] read " << rows1[0].size() << "\n";
	std::cout << "Rows read " << rows2.size() << "\n";
	std::cout << "Columns[2] read " << rows2[0].size() << "\n";

	// rows1[X][1], rows2[X][0] is constituency
	// rows1[0][X], rows2[0][X] is a header entry

	std::ofstream outputLog;
	outputLog.open("../outputLog.txt");

	//add header2 to header1
	for (size_t i = 2; i < rows2[0].size(); i++) {
		rows1[0].push_back(rows2[0][i]);
	}

	std::vector<bool> rows1taken(rows1.size(), false);
	std::vector<bool> rows2taken(rows2.size(), false);

	// First check for all those pairs of names which match EXACTLY

	for (size_t i = 1; i < rows1.size(); i++) {

		if (rows1taken[i]) {
			continue;
		}

		int bestJ = -1;
		for (size_t j = 1; j < rows2.size(); j++) {

			if (rows2taken[j]) {
				continue;
			}

			const std::string s1 = toUpper(rows1[i][1]);
			const std::string s2 = toUpper(rows2[j][0]);

			if (s1 == s2) {
				bestJ = j;
				break;
			}
		}

		if (bestJ == -1) {
			outputLog << "Couldn't find EXACT match for constituency " << rows1[i][1] << "\n";
			continue;
		}

		rows1taken[i] = true;
		rows2taken[bestJ] = true;

		outputLog << "Matched " << rows1[i][1] << " with " << rows2[bestJ][0] << " EXACTLY\n";

		for (size_t k = 2; k < rows2[0].size(); k++) {
			rows1[i].push_back(rows2[bestJ][k]);
		}
	}

	// now do INEXACT matching

	for (size_t j = 1; j < rows2.size(); j++) {
		if (rows2taken[j]) {
			continue;
		}

		int minDist = 2000;
		int bestI = 0;
		for (size_t i = 1; i < rows1.size(); i++) {

			if (rows1taken[i]) {
				continue;
			}

			const std::string s1 = toUpper(rows1[i][1]);
			const std::string s2 = toUpper(rows2[j][0]);

			int dist = compare(s1, s2);

			//outputLog << "S1 = " << s1 << " S2 = " << s2 << " | dist = " << dist << "\n";

			if (minDist > dist) {
				minDist = dist;
				bestI = i;
			}
		}

		if (minDist == 2000) {
			outputLog << "Couldn't find ANY match for constituency " << rows2[j][0] << "\n";
			continue;
		}

		if (minDist > 2) {
			outputLog << "Minimum distance is too large. D = " << minDist << ". S1 = "  << rows1[bestI][1] << " S2 = " << rows2[j][0] << " \n";
			continue;
		}

		rows1taken[bestI] = true; 
		rows2taken[j] = true; // not necessary to set this but for sake of completeness

		outputLog << "Matched " << rows1[bestI][1] << " with " << rows2[j][0] << " INEXACTLY with dist = " << minDist << " \n";

		for (size_t k = 2; k < rows2[0].size(); k++) {
			rows1[bestI].push_back(rows2[j][k]);
		}
	}

	outputLog << "\nUnmatched entries in File 1\n-------------------\n";

	for (int i = 1; i < rows1taken.size(); i++) {
		if (!rows1taken[i]) {
			outputLog << rows1[i][1] << "\n";
		}
	}

	outputLog << "\nUnmatched entries in File 2\n-------------------\n";

	for (int i = 1; i < rows2taken.size(); i++) {
		if (!rows2taken[i]) {
			outputLog << rows2[i][0] << "\n";
		}
	}

	outputLog.close();

	std::ofstream combined;
	combined.open("../combined_dp.csv");

	for (auto& row : rows1) {
		combined << row[0];
		for (size_t i = 1; i < rows1[0].size(); i++) {
			if (i < row.size()) {
				combined << "," << row[i];
			}
			else {
				combined << ",";
			}
		}
		combined << "\n";
	}

	combined.close();

	getchar();
	return 0;
}