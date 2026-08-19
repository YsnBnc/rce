#include "bridge.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
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

int client_class::client_side(int PORT, char TARGET_IP[16], uint32_t FILE_INDEX,
                              std::string FILE_NAME, std::string CMPL_COMMAND,
                              std::string FILE_CONTENT) {
  int client_socket = 0;
  struct sockaddr_in server_address;
  char buffer[1460] = {0};

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
  // FILE_CONTENT = catch_file(file_content);
  std::vector<uint8_t> payload =
      pack_file(FILE_INDEX, FILE_NAME, CMPL_COMMAND, FILE_CONTENT);
  std::cout << "Sent data: " << std::endl;
  for (int i = 0; i < payload.size(); i++) {
    std::cout << payload[i];
  }
  WireHeader header;
  header.payload_length = htonl(static_cast<uint8_t>(payload.size()));
  header.msg_type = htons(0x0100);
  send_exact(client_socket, &header, sizeof(WireHeader));    // Send header
  send_exact(client_socket, payload.data(), payload.size()); // Send payload

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
