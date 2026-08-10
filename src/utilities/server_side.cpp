#include <cstring>
#include <string>
#include <iostream>
#include "bridge.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif


int server_side(int PORT, std::string COMMAND)
{
    int hostSocket, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[2048] = {0};
    std::cout << "Server starting...";
#ifdef _WIN32
    WSAData wsaData{};
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        std::cerr << "WSAStartup() failed." << std::endl;
        return 1;
    }
#endif

    //Create socket
    hostSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (hostSocket < 0) {
        std::cerr << "Socket failed." << std::endl;
    }else {
        std::cout << "Socket created." << std::endl;
    }

    //Assign socket to a port
#ifdef _WIN32
    setsockopt(hostSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(hostSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    std::cout << PORT << std::endl;
    address.sin_port = htons(PORT);

    //Bind
    if (bind(hostSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed." << std::endl;
    }else{std::cerr << "Bind successful." << std::endl;}

    //Listen
    if (listen(hostSocket, 10) < 0) {
        std::cerr << "Listen failed." << std::endl;
    }else{std::cout << "Listening for connections..." << std::endl;}

    //Accept connections
    newSocket = accept(hostSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (newSocket < 0) {
        std::cerr << "Connecting to client failed." << std::endl;
    }else{std::cout << "Connection established." << std::endl;}

    //Read data and trigger compile phase
    std::string answer;
    int recieved_bytes = recv(newSocket, buffer, sizeof(buffer) -1, 0);
    if (recieved_bytes > 0) {
        std::cout << "File recieved" << std::endl;
        buffer[recieved_bytes] = '\0';
        std::cout << "Message from client:\n" << buffer << std::endl;
        catch_file(buffer);
        std::cout << COMMAND << std::endl;
        answer = compile_file(COMMAND);
        remove(FILENAME.c_str());
    }

    if (send(newSocket, answer.c_str(), answer.length(), 0) < 0) {
        std::cerr << "Error sending answer" << std::endl;
    }
    else {
        std::cout << "Answer sent" << std::endl;
    }

#ifdef _WIN32
    closesocket(newSocket);
    closesocket(hostSocket);
#else
    close(newSocket);
    close(hostSocket);
#endif
    return 0;
}