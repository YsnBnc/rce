#include <cstring>
#include <string>
#include <iostream>
#include "file_manager.h"
#include "bridge.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

using namespace std;

int server_side()
{
    int hostSocket, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[2048] = {0};

#ifdef _WIN32
    WSAData wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        cerr << "WSAStartup() failed." << endl;
        return 1;
    }
#endif

    //Create socket
    hostSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (hostSocket < 0) {
        cerr << "socket failed." << endl;
        exit(EXIT_FAILURE);
    }

    //Assign socket to a port
#ifdef _WIN32
    setsockopt(hostSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(hostSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //Bind
    if (bind(hostSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        cerr << "bind failed." << endl;
        exit(EXIT_FAILURE);
    }

    //Listen
    if (listen(hostSocket, 10) < 0) {
        cerr << "listen failed." << endl;
        exit(EXIT_FAILURE);
    }

    //Accept connections
    newSocket = accept(hostSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (newSocket < 0) {
        cerr << "accept failed." << endl;
        exit(EXIT_FAILURE);
    }

    //Read data and trigger compile phase
    string answer;
    int recieved_bytes = recv(newSocket, buffer, sizeof(buffer) -1, 0);
    if (recieved_bytes > 0) {
        buffer[recieved_bytes] = '\0';
        cout << "Message from client:\n" << buffer << endl;
        catch_file(buffer);
        answer = compile_file(COMMAND).c_str();
        remove("temp.py");
    }

    if (send(newSocket, answer.c_str(), answer.length(), 0) < 0) {
        cerr << "Error sending message" << endl;
        exit(EXIT_FAILURE);
    }
    else {
        cout << "Message sent" << endl;
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