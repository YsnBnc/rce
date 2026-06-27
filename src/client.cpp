#include <cstring>
#include <string>
#include <iostream>

#define PORT 8080

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
  const char *message = "Hello from client";

  //Create socket
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0) {
    cerr << "Error creating socket" << endl;
    exit(EXIT_FAILURE);
  }
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);

  //Convert IPv4 to IPv6
  if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
    cerr << "Invalid address/ Address not supported \n";
    exit(EXIT_FAILURE);
  }

  //Send data
  send(client_socket, message, strlen(message), 0);
  cout << "Message sent" << endl;

  //Close socket;
  close(client_socket);
  return 0;

}