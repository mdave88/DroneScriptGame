
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

string getFileName(const string& path) {
	string tmp = path.substr(path.find_last_of('/') + 1, path.size());
	return tmp.substr(0, tmp.find_first_of('.'));
}

string getExtension(const string& path) {
	return path.substr(path.find_last_of('.') + 1, path.size());
}