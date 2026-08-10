#include <iostream>
#include <bits/stdc++.h>
#include <sstream>
#include <fstream>
#include <array>
#include "bridge.h"

std::string FILENAME;

std::string read_file(const std::string& file_name) {
    std::cout << "Reading file" << std::endl;
    std::ifstream ifs(file_name);
    if (!ifs.is_open()) return "";
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

void catch_file(const char *buffer) {
    std::cout << "Catching file" << std::endl;
    std::ofstream file_to_compile(FILENAME);
    file_to_compile << buffer;
    file_to_compile.close();
}

std::string compile_file(const std::string& command) {
    std::cout << "Compiling file" << std::endl;
    std::array<char, 1024> buffer;
    std::string output;
    std::string redirect_command = command + " 2>&1";

#ifdef WIN32
    FILE* pipe = _popen(redirect_command.c_str(), "r");
#else
    FILE* pipe = popen(redirect_command.c_str(), "r");
#endif

    if (!pipe) std::cerr << "Failed to execution pipeline" << std::endl;

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }

#ifdef WIN32
    std::cout << "File compiled" << std::endl;
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return output;
}

