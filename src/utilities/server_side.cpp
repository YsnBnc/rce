#include "bridge.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

bool recv_exact(int fd, void *data, size_t size) {
  size_t rx = 0;
  auto *ptr = static_cast<const uint8_t *>(data);
  while (rx < size) {
    ssize_t res = recv_exact(fd, data, size);
    if (res <= 0)
      return false;
    rx += res;
  }
  return true;
}

int server_class::server_side(int PORT) {
  int hostSocket, newSocket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  // char filenameBuffer[256];
  char buffer[1460] = {0};
  std::cout << "Server starting...";
#ifdef _WIN32
  WSAData wsaData{};
  int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (wsaResult != 0) {
    std::cerr << "WSAStartup() failed." << std::endl;
    return 1;
  }
#endif

  // Create socket
  hostSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (hostSocket < 0) {
    std::cerr << "Socket failed." << std::endl;
  } else {
    std::cout << "Socket created." << std::endl;
  }

  // Assign socket to a port
#ifdef _WIN32
  setsockopt(hostSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt,
             sizeof(opt));
#else
  setsockopt(hostSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  std::cout << PORT << std::endl;
  address.sin_port = htons(PORT);

  // Bind
  if (bind(hostSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
    std::cerr << "Bind failed." << std::endl;
  } else {
    std::cerr << "Bind successful." << std::endl;
  }

  // Listen
  if (listen(hostSocket, 10) < 0) {
    std::cerr << "Listen failed." << std::endl;
  } else {
    std::cout << "Listening for connections..." << std::endl;
  }

  // Accept connections
  newSocket =
      accept(hostSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen);
  if (newSocket < 0) {
    std::cerr << "Connecting to client failed." << std::endl;
  } else {
    std::cout << "Connection established." << std::endl;
  }

  // Recieve data
  WireHeader header;
  if (!recv_exact(newSocket, &header, sizeof(WireHeader))) {
    std::cerr << "Error recieving header.";
  }
  uint32_t payload_length = ntohl(header.payload_length);
  constexpr uint32_t MAX_PAYLOAD_SIZE = 64 * 1024 * 1024;
  if (payload_length > MAX_PAYLOAD_SIZE) {
    std::cerr << "Payload length exceed limit.";
  }

  std::vector<uint8_t> payload_buffer(payload_length);
  if (!recv_exact(newSocket, payload_buffer.data(), payload_length)) {
    std::cerr << "Error recieving data.";
  }

  // Deserialize and assign them to global variables
  size_t offset = 0;
  auto can_read = [&](size_t bytes) {
    return (offset + bytes) <= payload_buffer.size();
  };
  // FILE_INDEX
  uint32_t net_id;
  if (!can_read(4)) {
    std::cerr << "Unable to read index";
    return false;
  }
  std::memcpy(&net_id, payload_buffer.data() + offset, 4);
  FILE_INDEX = ntohl(net_id);
  offset += 4;

  // FILE_NAME
  uint16_t net_fn_len;
  if (!can_read(2)) {
    std::cerr << "Unable to get file name length.";
    return false;
  }
  std::memcpy(&net_fn_len, payload_buffer.data() + offset, 2);
  uint16_t fn_len = ntohs(net_fn_len);
  if (can_read(fn_len)) {
    std::cerr << "Unable to read file name.";
    return false;
  }
  offset += 2;
  FILE_NAME.assign(
      reinterpret_cast<const char *>(payload_buffer.data() + offset), fn_len);
  offset += fn_len;

  // COMPILE_COMMAND
  uint16_t net_cmd_len;
  if (!can_read(2)) {
    std::cerr << "Unable to get command length.";
    return false;
  }
  std::memcpy(&net_cmd_len, payload_buffer.data() + offset, 2);
  uint16_t cmd_len = ntohs(net_cmd_len);
  if (can_read(cmd_len)) {
    std::cerr << "Unable to read command.";
    return false;
  }
  offset += 2;
  COMPILE_COMMAND.assign(
      reinterpret_cast<const char *>(payload_buffer.data() + offset), cmd_len);
  offset += cmd_len;

  // FILE_CONTENT
  if (!can_read(4)) {
    std::cerr << "Unable to get file content length.";
    return false;
  }
  uint32_t net_ct_len;
  std::memcpy(&net_ct_len, payload_buffer.data() + offset, 4);
  uint32_t ct_len = ntohl(net_ct_len);
  if (can_read(fn_len)) {
    std::cerr << "Unable to read file content.";
    return false;
  }
  offset += 4;
  FILE_CONTENT.assign(payload_buffer.data() + offset,
                      payload_buffer.data() + offset + ct_len);

#ifdef _WIN32
  closesocket(newSocket);
  closesocket(hostSocket);
#else
  close(newSocket);
  close(hostSocket);
#endif
  return 0;
}
