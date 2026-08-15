#include "bridge.h"
#include <array>
#include <bits/stdc++.h>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>

std::string read_file(const std::string &file_name) {
  std::cout << "Reading file" << std::endl;
  std::ifstream ifs(file_name);
  if (!ifs.is_open())
    return "";
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

std::vector<int> pull_index(const std::string &message) {
  std::vector<int> indexValues;
  const std::string delimeter = "~~~";

  size_t startPoint = message.find(delimeter);
  if (startPoint == std::string::npos)
    return indexValues; // No start point

  startPoint += delimeter.length();
  size_t endPoint = message.find(delimeter, startPoint);
  if (endPoint == std::string::npos)
    return indexValues; // No end point

  std::string content = message.substr(startPoint, endPoint - startPoint);

  std::stringstream ss(content);
  std::string token;
  while (std::getline(ss, token, ',')) {
    if (!token.empty()) {
      indexValues.push_back(std::stoi(token));
    }
  }
  return indexValues;
}

std::string compile_file(const std::string &command) {
  std::cout << "Compiling file" << std::endl;
  std::array<char, 1024> buffer;
  std::string output;
  std::string redirect_command = command + " 2>&1";

#ifdef WIN32
  FILE *pipe = _popen(redirect_command.c_str(), "r");
#else
  FILE *pipe = popen(redirect_command.c_str(), "r");
#endif

  if (!pipe)
    std::cerr << "Failed to execution pipeline" << std::endl;

  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    output += buffer.data();
  }
  std::cout << "File compiled" << std::endl;
#ifdef WIN32
  _pclose(pipe);
#else
  pclose(pipe);
#endif
  return output;
}
