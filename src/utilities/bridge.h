#ifndef RCE_BRIDGE_H
#define RCE_BRIDGE_H
#include <cstdint>
#include <string>
#include <vector>

// extern int PORT;
// extern char TARGET_IP[16];
// int indexing(std::string text, int start_index);
std::vector<int> pull_index(const std::string &message);
std::string read_file(const std::string &file_name);
void catch_file(const char *buffer);
std::string compile_file(const std::string &command);
extern std::string PATH_TO_FILE;
extern std::string FILENAME;
int client_side(int PORT, char TARGET_IP[16]);
int server_side(int PORT, std::string COMMAND);

#pragma pack(push, 1)
struct PacketHeader {
  uint32_t packetID;
  uint32_t payloadLength;
};
#pragma pack(pop)

#endif // RCE_BRIDGE_H
