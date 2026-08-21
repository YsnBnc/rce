#include "bridge.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <ostream>
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

int server_class::server_side(int PORT)
{
  int hostSocket, newSocket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  uint32_t FILE_INDEX;
  std::string FILE_NAME;
  std::string FILE_CONTENT;
  std::string PATH_TO_FILE;
  std::string COMPILE_COMMAND;

  std::cout << "Server starting...";
#ifdef _WIN32
  WSAData wsaData{};
  int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (wsaResult != 0)
  {
    std::cerr << "WSAStartup() failed." << std::endl;
    return 1;
  }
#endif

  // Create socket
  hostSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (hostSocket < 0)
  {
    std::cerr << "Socket failed." << std::endl;
    return 1;
  }
  else
  {
    std::cout << "Socket created." << std::endl;
  }

  // Assign socket to a port
#ifdef _WIN32
  setsockopt(hostSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
  setsockopt(hostSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  std::cout << PORT << std::endl;
  address.sin_port = htons(PORT);

  // Bind
  if (bind(hostSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    std::cerr << "Bind failed." << std::endl;
    return 1;
  }
  else
  {
    std::cerr << "Bind successful." << std::endl;
  }

  // Listen
  if (listen(hostSocket, 10) < 0)
  {
    std::cerr << "Listen failed." << std::endl;
    return 1;
  }
  else
  {
    std::cout << "Listening for connections..." << std::endl;
  }
  while (true)
  {
    // Accept connections
    newSocket = accept(hostSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (newSocket < 0)
    {
      std::cerr << "Connecting to client failed." << std::endl;
      continue;
    }
    else
    {
      std::cout << "Connection established." << std::endl;
    }

    auto handler = [&]() -> bool {
      // Recieve data
      WireHeader header;
      if (!recv_exact(newSocket, &header, sizeof(WireHeader)))
      {
        std::cerr << "Error recieving header.";
        return false;
      }
      uint32_t payload_length = ntohl(header.payload_length);
      constexpr uint32_t MAX_PAYLOAD_SIZE = 64 * 1024 * 1024;
      if (payload_length > MAX_PAYLOAD_SIZE)
      {
        std::cerr << "Payload length exceed limit." << std::endl;
        ;
        return false;
      }

      std::vector<uint8_t> payload_buffer(payload_length);
      if (!recv_exact(newSocket, payload_buffer.data(), payload_length))
      {
        std::cerr << "Error recieving data.";
        return false;
      }
      else
      {
        std::cout << "Data recieved." << std::endl;
      }

      // Deserialize and assign them to global variables
      size_t offset = 0;
      auto can_read = [&](size_t bytes)
      {
        return (offset + bytes) <= payload_buffer.size();
      };
      // FILE_INDEX
      if (!can_read(4))
      {
        std::cerr << "Unable to read index" << std::endl;
        ;
        return false;
      }
      uint32_t net_id;
      std::memcpy(&net_id, payload_buffer.data() + offset, 4);
      FILE_INDEX = ntohl(net_id);
      offset += 4;

      // FILE_NAME
      if (!can_read(2))
      {
        std::cerr << "Unable to get file name length." << std::endl;
        ;
        return false;
      }
      uint16_t net_fn_len;
      std::memcpy(&net_fn_len, payload_buffer.data() + offset, 2);
      offset += 2;
      uint16_t fn_len = ntohs(net_fn_len);
      if (!can_read(fn_len))
      {
        std::cerr << "Unable to read file name." << std::endl;
        ;
        return false;
      }
      FILE_NAME.assign(reinterpret_cast<const char *>(payload_buffer.data() + offset), fn_len);
      offset += fn_len;
      std::cout << "Received file: " << FILE_NAME << std::endl;

      // COMPILE_COMMAND
      if (!can_read(2))
      {
        std::cerr << "Unable to get command length." << std::endl;
        ;
        return false;
      }
      uint16_t net_cmd_len;
      std::memcpy(&net_cmd_len, payload_buffer.data() + offset, 2);
      offset += 2;
      uint16_t cmd_len = ntohs(net_cmd_len);
      if (!can_read(cmd_len))
      {
        std::cerr << "Unable to read command." << std::endl;
        ;
        return false;
      }
      COMPILE_COMMAND.assign(reinterpret_cast<const char *>(payload_buffer.data() + offset), cmd_len);
      offset += cmd_len;

      // FILE_CONTENT
      if (!can_read(4))
      {
        std::cerr << "Unable to get file content length." << std::endl;
        ;
        return false;
      }
      uint32_t net_ct_len;
      std::memcpy(&net_ct_len, payload_buffer.data() + offset, 4);
      offset += 4;
      uint32_t ct_len = ntohl(net_ct_len);
      if (!can_read(ct_len))
      {
        std::cerr << "Unable to read file content." << std::endl;
        ;
        return false;
      }
      FILE_CONTENT.assign(reinterpret_cast<const char *>(payload_buffer.data() + offset), ct_len);

      // Compile recieved data and send reponse
      write_file(FILE_NAME, FILE_CONTENT);
      std::vector<uint8_t> response_payload = pack_response(FILE_INDEX, compile_file(COMPILE_COMMAND));

      header.payload_length = htonl(static_cast<uint32_t>(response_payload.size()));
      header.msg_type = htons(0x0200);
      if (!send_exact(newSocket, &header, sizeof(WireHeader)))
      {
        std::cerr << "Error sending on response header.";
        return false;
      } // Send header
      if (!send_exact(newSocket, response_payload.data(), response_payload.size()))
      {
        std::cerr << "Error sending in response payload." << std::endl;
        remove(FILE_NAME.c_str());
        return false;
      }
      else
      {
        std::cout << "Response payload sent." << std::endl;
        remove(FILE_NAME.c_str());
        return true;
      }
    };
    if(!handler()){
      std::cerr << "Client connection dropped or failed." << std::endl;
    }
#ifdef _WIN32
    closesocket(newSocket);
#else
    close(newSocket);
#endif
  }

#ifdef _WIN32
  closesocket(hostSocket);
#else
  close(hostSocket);
#endif
  return 0;
} 