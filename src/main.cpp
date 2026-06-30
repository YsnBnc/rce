#include <iostream>
#include "utilities/bridge.h"
using namespace std;

int PORT;
char TARGET_IP[16];
std::string COMMAND;
std::string PATH_TO_FILE;

int client() {
    cout << "Port: ";
    cin >> PORT;
    cout <<"Server IP: ";
    cin >> TARGET_IP;
    cout <<"File: ";
    cin >> PATH_TO_FILE;
    cout <<"Client is executing...\n";
    client_side();
    return 0;
}
int server() {
    cout << "Port: ";
    cin >> PORT;
    cout <<"Command you want to execute: ";
    cin >> COMMAND;
    cout <<"Server is executing...\n";
    server_side();
    return 0;
}

int main() {
    int side = 0;
    cout << "Remote Code Execution\n" << "Specify a side\n 1. Server\n 2. Client\n";
    cin >> side;
    switch (side) {
        case 1:
            server();
            return 0;
        case 2:
            client();
            return 0;
        default:
            cout << "Invalid command\n";
            return 1;
    }
}

