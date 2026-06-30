#include <iostream>
#include <bits/stdc++.h>
#include <sstream>
#include <fstream>
#include <array>
#include <cstdlib>
#include "file_manager.h"
using namespace std;

string read_file(const string& file_name) {
    ifstream ifs(file_name);
    if (!ifs.is_open()) return "";
    stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

void catch_file(const char *buffer) {
    ofstream file_to_compile("temp.py");
    file_to_compile << buffer;
    file_to_compile.close();
}

string compile_file(const string& command) {
    array<char, 1024> buffer;
    string output;
    string redirect_command = command + " 2>&1";

#ifdef WIN32
    FILE* pipe = _popen(redirect_command.c_str(), "r");
#else
    FILE* pipe = popen(redirect_command.c_str(), "r");
#endif

    if (!pipe) cerr << "Failed to execution pipeline" << endl;

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }

#ifdef WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return output;
}