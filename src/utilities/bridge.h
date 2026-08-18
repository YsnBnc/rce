#ifndef RCE_BRIDGE_H
#define RCE_BRIDGE_H
#include <cstdint>
#include <filesystem>
#include <stdint.h>
#include <string>
#include <vector>

class client_class {
public:
  int client_side(int PORT, char *TARGET_IP, std::string COMPILE_COMMAND);
};
class server_class {
public:
  int server_side(int PORT);
};
extern uint32_t FILE_INDEX;
extern std::string FILE_NAME;
extern std::string FILE_CONTENT;
extern std::string PATH_TO_FILE;
extern std::string COMPILE_COMMAND;
std::string read_file(const std::filesystem::path &PATH_TO_FILE);
void catch_file(std::string FILE_CONTENT);
int client_side(int PORT, char TARGET_IP[16]);
int server_side(int PORT);
std::vector<uint8_t> pack_file();
#endif                // RCE_BRIDGE_H
#pragma pack(push, 1) // Use 1 bit alignment forecefully
struct WireHeader {
  uint32_t payload_length;
  uint16_t msg_type; // Message ID
};
#pragma pack(pop) // Return default
