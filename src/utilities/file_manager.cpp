#include "bridge.h"
#include <array>
#include <bits/stdc++.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

uint32_t FILE_INDEX;
std::string FILE_NAME;
std::string COMPILE_COMMAND;
std::string FILE_CONTENT;

std::string read_file(const std::string &file_name) {
  std::cout << "Reading file" << std::endl;
  std::ifstream ifs(file_name);
  if (!ifs.is_open())
    return "";
  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

void catch_file(std::string PATH_TO_FILE) {
  std::cout << "Catching file" << std::endl;
  std::ofstream file(FILE_NAME);
  file << PATH_TO_FILE;
  file.close();
}

std::vector<uint8_t> pack_file() {
  std::vector<uint8_t> buffer;
  // Index
  uint32_t net_id = htonl(FILE_INDEX);
  const auto *p = reinterpret_cast<const uint8_t *>(&net_id);
  buffer.insert(buffer.end(), p, p + 4); // 4 bytes
  // Filename
  if (FILE_NAME.size() > UINT16_MAX) {
    std::cerr << "Unable to send. Filename is too big.";
    return {};
  }
  uint16_t net_name_length = htons(static_cast<uint16_t>(FILE_NAME.size()));
  p = reinterpret_cast<const uint8_t *>(&net_name_length);
  buffer.insert(buffer.end(), p, p + 2); // 2 bytes
  buffer.insert(buffer.end(), FILE_NAME.begin(), FILE_NAME.end());
  // Compile command
  if (COMPILE_COMMAND.size() > UINT16_MAX) {
    std::cerr << "Unable to send. Entered command is too long.";
    return {};
  }
  uint16_t net_cmd_length =
      htons(static_cast<uint16_t>(COMPILE_COMMAND.size()));
  p = reinterpret_cast<const uint8_t *>(&net_cmd_length);
  buffer.insert(buffer.end(), p, p + 2); // 2 bytes
  buffer.insert(buffer.end(), COMPILE_COMMAND.begin(), COMPILE_COMMAND.end());
  // File content
  if (FILE_CONTENT.size() > UINT32_MAX) {
    std::cerr << "Unable to send. File exceeds 4gb limit.";
    return {};
  }
  uint32_t net_ct_len = htonl(static_cast<uint32_t>(FILE_CONTENT.size()));
  p = reinterpret_cast<const uint8_t *>(&net_ct_len);
  buffer.insert(buffer.end(), p, p + 4); // 4 bytes
  buffer.insert(buffer.end(), FILE_CONTENT.begin(), FILE_CONTENT.end());

  return buffer;
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
