#ifndef RCE_BRIDGE_H
#define RCE_BRIDGE_H
#include <string>

//extern int PORT;
//extern char TARGET_IP[16];
std::string read_file(const std::string & file_name);
void catch_file(const char *buffer);
std::string compile_file(const std::string & command);
extern std::string PATH_TO_FILE;
extern std::string FILENAME;
int client_side(int PORT, char TARGET_IP[16]);
int server_side(int PORT, std::string COMMAND);

#endif //RCE_BRIDGE_H
