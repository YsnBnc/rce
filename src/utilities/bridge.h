#ifndef RCE_BRIDGE_H
#define RCE_BRIDGE_H
#include <cstdint>
#include <filesystem>
#include <stdint.h>
#include <string>
#include <vector>

class client_class {
public:
  int client_side(int PORT, char TARGET_IP[16], uint32_t FILE_INDEX,std::string FILE_NAME, std::string COMPILE_COMMAND,std::string FILE_CONTENT);
};
class server_class {
public:
  int server_side(int PORT);
};
std::string read_file(const std::filesystem::path &PATH_TO_FILE);
void write_file(std::string FILE_NAME,std::string FILE_CONTENT);
bool send_exact(int fd, const void *data, size_t size);
bool recv_exact(int fd, void *data, size_t size);
uint32_t next_id();
std::string compile_file(const std::string &command);
std::vector<uint8_t> pack_file(uint32_t FILE_INDEX, std::string FILE_NAME,std::string CMPL_COMMAND,std::string FILE_CONTENT);
std::vector<uint8_t> pack_response(uint32_t RESPONSE_INDEX, std::string RESPONSE);
#endif                // RCE_BRIDGE_H

#pragma pack(push, 1) // Use 1 bit alignment forecefully
struct WireHeader {
  uint32_t payload_length;
  uint16_t msg_type; // Message ID
};
#pragma pack(pop) // Return default
