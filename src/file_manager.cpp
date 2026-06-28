#include <iostream>
#include <bits/stdc++.h>
#include <sstream>
#include "file_manager.h"
using namespace std;


string read_file(const string& file_name) {
    ifstream ifs(file_name);
    if (!ifs.is_open()) return "";
    stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}