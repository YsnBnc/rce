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

int client_class::client_side(int PORT, char TARGET_IP[16], uint32_t FILE_INDEX,
                              std::string FILE_NAME, std::string CMPL_COMMAND,
                              std::string FILE_CONTENT) {
  int client_socket = 0;
  struct sockaddr_in server_address;
  uint32_t RESPONSE_INDEX;
  std::string RESPONSE;

#ifdef WIN32
  WSADATA wsaData;
  int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (wsaResult != 0) {
    std::cerr << "WSAStartup() failed." << std::endl;
    return 1;
  }
#endif

  // Create socket
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0) {
    std::cerr << "Error creating socket" << std::endl;
  }
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);

  // Convert IPv4 to IPv6
  if (inet_pton(AF_INET, TARGET_IP, &server_address.sin_addr) <= 0) {
    std::cerr << "Invalid address/ Address not supported."<< std::endl;;
  }

  if (connect(client_socket, (struct sockaddr *)&server_address,sizeof(server_address)) < 0) {
#ifdef WIN32
    closesocket(client_socket);
    WSACleanup();
    std::cerr << "Connect failed with error code: " << WSAGetLastError() << std::endl;
#else
    close(client_socket);
    std::cerr << "Connect failed with error code: " << strerror(errno) << std::endl;
#endif
  }

  // Manage packet content
  std::vector<uint8_t> payload = pack_file(FILE_INDEX, FILE_NAME, CMPL_COMMAND, FILE_CONTENT);
  std::cout << "Sent data: " << std::endl;

  WireHeader header;
  header.payload_length = htonl(static_cast<uint8_t>(payload.size()));
  header.msg_type = htons(0x0100);
  send_exact(client_socket, &header, sizeof(WireHeader));    // Send header
  send_exact(client_socket, payload.data(), payload.size()); // Send payload
  std::cout << "Sent file: " << FILE_NAME << std::endl;

  // Recieve answer data
  if (!recv_exact(client_socket, &header, sizeof(WireHeader))) {
    std::cerr << "Error recieving header." << std::endl;
    return 1;
  } 
  uint32_t payload_length = ntohl(header.payload_length);
  constexpr uint32_t MAX_PAYLOAD_SIZE = 64 * 1024 * 1024;
  if (payload_length > MAX_PAYLOAD_SIZE) {
    std::cerr << "Payload length exceed limit."<< std::endl;;
    return 1;
  }

  std::vector<uint8_t> payload_buffer(payload_length);
  if (!recv_exact(client_socket, payload_buffer.data(), payload_length)) {
    std::cerr << "Error recieving data."<< std::endl;
    return 1;
  } else {
    std::cout << "Data recieved." << std::endl;
  }

  size_t offset = 0;
  auto can_read = [&](size_t bytes) {
    return (offset + bytes) <= payload_buffer.size();
  };
  //RESPONSE_INDEX
  if (!can_read(4)) {
    std::cerr << "Unable to read index"<< std::endl;;
    return false;
  }
  uint32_t net_id;
  std::memcpy(&net_id, payload_buffer.data() + offset, 4);
  RESPONSE_INDEX = ntohl(net_id);
  offset += 4;
  //std::cout << RESPONSE_INDEX << std::endl;

  //RESPONSE
  if (!can_read(2)) {
    std::cerr << "Unable to get file name length."<< std::endl;;
    return false;
  }
  uint16_t net_rsp_len;
  std::memcpy(&net_rsp_len, payload_buffer.data() + offset, 2);
  offset += 2;
  uint16_t rsp_len = ntohs(net_rsp_len);
  if (!can_read(rsp_len)) {
    std::cerr << "Unable to read file name.";
    return false;
  }
  RESPONSE.assign(reinterpret_cast<const char *>(payload_buffer.data() + offset), rsp_len);
  offset += rsp_len;
  std::cout << "Response from server:\n" << RESPONSE << std::endl;


// Close socket;
#ifdef _WIN32
  closesocket(client_socket);
  WSACleanup();
#else
  close(client_socket);
#endif
  return 0;
}
