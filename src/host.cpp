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
#endif

using namespace std;

int main()
{
    // creating socket
    int hostSocket = socket(AF_INET, SOCK_STREAM, 0);

    // specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // binding socket.
    bind(hostSocket, (struct sockaddr*)&serverAddress,
         sizeof(serverAddress));

    // listening to the assigned socket
    listen(hostSocket, 5);

    // accepting connection request
    int clientSocket= accept(hostSocket, nullptr, nullptr);
    // sending data
    while (true)
    {
        char buffer[1024] = { 0 };
        recv(clientSocket, buffer, sizeof(buffer), 0);
        cout << "Message from client: " << buffer << endl;
    }

    // closing socket
    #ifdef _WIN32
    closesocket(hostSocket);
    #else
    close(hostSocket);
    #endif

    return 0;
}