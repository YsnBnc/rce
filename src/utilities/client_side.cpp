#include "bridge.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <wx/wx.h>

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib") // Links the winsock library
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

using namespace std;

int client_side(int PORT, char TARGET_IP[16]) {
  string MESSAGE;
  int client_socket = 0;
  struct sockaddr_in server_address;
  char buffer[1460] = {0};
  const string file_content =
      read_file(PATH_TO_FILE); // TODO This wants absolute path
  if (file_content.empty()) {
    std::cerr << "Client failed to read file data." << std::endl;
    return 1;
  }

#ifdef WIN32
  WSADATA wsaData;
  int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (wsaResult != 0) {
    cerr << "WSAStartup() failed." << endl;
    return 1;
  }
#endif

  // Create socket
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0) {
    cerr << "Error creating socket" << endl;
  }
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);

  // Convert IPv4 to IPv6
  if (inet_pton(AF_INET, TARGET_IP, &server_address.sin_addr) <= 0) {
    cerr << "Invalid address/ Address not supported \n";
  }

  if (connect(client_socket, (struct sockaddr *)&server_address,
              sizeof(server_address)) < 0) {
#ifdef WIN32
    WSACleanup();
    cerr << "Connect failed with error code: " << WSAGetLastError() << endl;
#else
    cerr << "Connect failed with error code: " << strerror(errno) << endl;
#endif
  }

  // Manage packet content
  int start_index = 0;
  int end_index;
  vector<vector<int>> index;
  std::ostringstream oss;
  vector<string> msg_packet = {FILENAME, file_content};
  for (int i = 0; i < std::size(msg_packet); i++) {
    end_index = start_index + msg_packet[i].length() - 1;
    index.push_back({start_index, end_index});
    start_index = end_index + 1;
    MESSAGE += msg_packet[i];
  }
  for (const auto &pair : index) {
    oss << pair[0] << "," << pair[1] << ",";
  }
  MESSAGE += "~~~" + oss.str() + "~~~";
  // cout << MESSAGE << endl;

  // Send package content
  if (send(client_socket, MESSAGE.c_str(), MESSAGE.size(), 0) < 0) {
    cerr << "Error sending request" << endl;
  } else {
    cout << "Request sent" << endl;
    cout << msg_packet.data() << endl;
  }

  // Recieve answer
  char answer_bytes[2048];
  int recieved_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
  if (recieved_bytes > 0) {
    answer_bytes[recieved_bytes] = '\0';
    cout << "Answer from server: " << buffer << endl;
  } else {
    cerr << "Error receiving answer" << endl;
  }

  // Close socket;
#ifdef _WIN32
  closesocket(client_socket);
  WSACleanup();
#else
  close(client_socket);
#endif
  return 0;
}
