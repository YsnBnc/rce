#ifndef RCE_BRIDGE_H
#define RCE_BRIDGE_H
#include <string>
#include <wx/wx.h>

//extern int PORT;
//extern char TARGET_IP[16];
extern std::string PATH_TO_FILE;
extern std::string COMMAND;
int client_side(int PORT, char TARGET_IP[16]);
int server_side(int PORT);

#endif //RCE_BRIDGE_H
