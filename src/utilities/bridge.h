#ifndef RCE_BRIDGE_H
#define RCE_BRIDGE_H
#include <string>

extern int PORT;
extern char TARGET_IP[16];
extern std::string PATH_TO_FILE;
extern std::string COMMAND;
int client_side();
int server_side();

#endif //RCE_BRIDGE_H
