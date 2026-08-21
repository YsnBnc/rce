#include "bridge.h"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>
#include <atomic>
#ifdef WIN32
#else
#include <netinet/in.h>
#endif // WIN32

bool recv_exact(int fd, void *data, size_t size) {
  size_t rx = 0;
  auto *ptr = static_cast<uint8_t *>(data);
  while (rx < size) {
    ssize_t res = recv(fd, (ptr + rx), (size - rx), 0);
    if (res <= 0)
      return false;
    rx += res;
  }
  return true;
}

bool send_exact(int fd, const void *data, size_t size) {
  size_t sent = 0;
  const auto *ptr = static_cast<const uint8_t *>(data);
  while (sent < size) {
    ssize_t res = send(fd, ptr + sent, size - sent, 0);
    if (res <= 0)
      return false; // Socket error
    sent += res;
  }
  return true;
}


std::string read_file(const std::filesystem::path &PATH_TO_FILE) {
  std::cout << "Reading file" << std::endl;
  std::ifstream ifs(PATH_TO_FILE, std::ios::in | std::ios::binary);
  if (!ifs.is_open())
    return "";
  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

void write_file(std::string FILE_NAME, std::string FILE_CONTENT) {
  std::cout << "Catching file" << std::endl;
  std::ofstream file(FILE_NAME);
  file << FILE_CONTENT;
  file.close();
}

uint32_t next_id(){
  static std::atomic<uint32_t> global_counter{1};
  return global_counter.fetch_add(1, std::memory_order_relaxed);
}

std::vector<uint8_t> pack_file(uint32_t FILE_INDEX, std::string FILE_NAME,std::string CMPL_COMMAND,std::string FILE_CONTENT) {
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
  if (CMPL_COMMAND.size() > UINT16_MAX) {
    std::cerr << "Unable to send. Entered command is too long.";
    return {};
  }
  uint16_t net_cmd_length = htons(static_cast<uint16_t>(CMPL_COMMAND.size()));
  p = reinterpret_cast<const uint8_t *>(&net_cmd_length);
  buffer.insert(buffer.end(), p, p + 2); // 2 bytes
  buffer.insert(buffer.end(), CMPL_COMMAND.begin(), CMPL_COMMAND.end());
  // File content
  if (FILE_CONTENT.size() > UINT32_MAX) {
    std::cerr << "Unable to send. File exceeds 4gb limit.";
    return {};
  }
  uint32_t net_ct_len = htonl(static_cast<uint32_t>(FILE_CONTENT.size()));
  p = reinterpret_cast<const uint8_t *>(&net_ct_len);
  buffer.insert(buffer.end(), p, p + 4); // 4 bytes

  buffer.insert(buffer.end(), FILE_CONTENT.data(),
                FILE_CONTENT.data() + FILE_CONTENT.size());

  return buffer;
}

std::vector<uint8_t> pack_response(uint32_t RESPONSE_INDEX, std::string RESPONSE){
  std::vector<uint8_t> buffer;
  //Index
  uint32_t net_id = htonl(RESPONSE_INDEX);
  const auto *p = reinterpret_cast<const uint8_t *>(&net_id);
  buffer.insert(buffer.end(), p, p + 4); 
  //Response
  uint16_t net_rsp_length = htons(static_cast<uint16_t>(RESPONSE.size()));
  p = reinterpret_cast<const uint8_t *>(&net_rsp_length);
  buffer.insert(buffer.end(), p, p + 2); // 2 bytes
  buffer.insert(buffer.end(), RESPONSE.begin(), RESPONSE.end());

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

  if (!pipe){
    return "Failed to execution pipeline.\n";
  }
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
