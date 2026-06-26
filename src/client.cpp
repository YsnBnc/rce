#include <cstring>
#include <string>
#include <iostream>

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
    #ifdef _WIN32
    // 1. Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Winsock initialization failed.\n";
        return 1;
    }
    #endif

    // 2. Creating socket
    // On Windows, sockets are UINT_PTR, but keeping 'int' works for basic setups
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cout << "Socket creation failed.\n";
        #ifdef _WIN32 aging WSACleanup(); 
        #endif
        return 1;
    }

    // 3. Specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);

    // REPLACE THIS string with your actual Linux machine IP address
    #ifdef _WIN32
    inet_pton(AF_INET, "192.168.0.26", &serverAddress.sin_addr);
    #else
    inet_pton(AF_INET, "192.168.0.12", &serverAddress.sin_addr);
    #endif

    // 4. Sending connection request
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cout << "Connection failed.\n";
        #ifdef _WIN32
        closesocket(clientSocket);
        #else
        close(clientSocket);
        #endif

        #ifdef _WIN32 
        WSACleanup(); 
        #endif
        return 1;
    }

    while (true)
    {
        string message;
        cin >> message;
        send(clientSocket, message.c_str(), message.size(), 0);
    }

    #ifdef _WIN32
    closesocket(clientSocket);
    #else
    close(clientSocket);
    #endif

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}