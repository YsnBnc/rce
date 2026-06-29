#include <cstring>
#include <string>
#include <iostream>
#include "file_manager.h"

#define PORT 9999
#define TARGET_IP "192.168.0.23"

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib") // Links the winsock library
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

using namespace std;

int main()
{
  int client_socket = 0;
  struct sockaddr_in server_address;
  char buffer[1024] = {0};
  //TODO Absolute path is quite bad
  const string message = read_file("C:/Users/asuma/CLionProjects/rpc/src/test.py");
  if (message.empty()) {
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

  //Create socket
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0) {
    cerr << "Error creating socket" << endl;
    exit(EXIT_FAILURE);
  }
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);

  //Convert IPv4 to IPv6
  if(inet_pton(AF_INET, TARGET_IP, &server_address.sin_addr) <= 0) {
    cerr << "Invalid address/ Address not supported \n";
    exit(EXIT_FAILURE);
  }

  if (connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
#ifdef WIN32
    WSACleanup();
    cerr << "Connect failed with error code: " << WSAGetLastError() << endl;
#else
    cerr << "Connect failed with error code: " << strerror(errno) << endl;
#endif
    exit(EXIT_FAILURE);
  }

  //Send data
  if (send(client_socket, message.c_str(), message.size(), 0) < 0) {
    cerr << "Error sending message" << endl;
    exit(EXIT_FAILURE);
  }
  else {
    cout << "Message sent" << endl;
    cout << "DEBUG: Sending bytes: " << message << endl;
    cout << "DEBUG: Sending bytes size: " << message.size() << endl;
  }

  //Recieve answer
  char answer_bytes[2048];
  int recieved_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
  if (recieved_bytes > 0) {
    answer_bytes[recieved_bytes] = '\0';
    cout << "Message from server: " << buffer << endl;
  }
  else {
    cerr << "Error receiving message" << endl;
    exit(EXIT_FAILURE);
  }

  //Close socket;
#ifdef _WIN32
  closesocket(client_socket);
  WSACleanup();
#else
  close(client_socket);
#endif
  return 0;

}