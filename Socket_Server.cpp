#include <iostream>
#include <winsock2.h>
// for the system time
#include <chrono>
#include <ctime>
#include <iomanip>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

// basic function of contact:
// 1. mutilple server
// 2. caesar encrypt
// 3. log record

// Caesar Cipher encryption function
string CaesarEncrypt(const string &text, int shift)
{
    string result = text;
    for (char &c : result)
    {
        if (isalpha(c))
        {
            char base = islower(c) ? 'a' : 'A';
            c = (c - base + shift) % 26 + base;
        }
    }
    return result;
}

tm getTime()
{
    auto clock_now = chrono::system_clock::now();
    time_t now_time_t = chrono::system_clock::to_time_t(clock_now);

    return *localtime(&now_time_t);
}

int main()
{
    WSADATA wsaData;
    SOCKET serverSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    tm now_tm;

    // Open log file
    ofstream logFile("socket_log.txt", ios::app); // Append mode
    if (!logFile.is_open())
    {
        cerr << "Failed to open log file" << endl;
        return 1;
    }

    // initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
    {
        now_tm = getTime();
        cerr << "WSAStartup failed" << endl;
        logFile << "WSAStartup failed " << put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << endl;
        return 1;
    }

    // create socket
    // udp, and default protocal ipproto
    if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        now_tm = getTime();
        cerr << "Socket creation failed" << endl;
        logFile << "Server_Socket Creation failed " << put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << endl;
        WSACleanup();
        logFile.close();
        return 1;
    }
    else
    {
        now_tm = getTime();
        logFile << "---------------------------------------------" << endl;
        logFile << "Server_Socket Created on " << put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << endl;
    }

    // server adress
    serverAddr.sin_family = AF_INET;                   // as ipv4
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP); // convert to binary and send the ip
    serverAddr.sin_port = htons(PORT);                 // port that the message arrive

    // read from command line and send
    cout << "\033[31m" << "\033[2m";
    cout << "Type exit to exit" << endl;
    cout << "\033[0m";
    while (1)
    {
        cout << "\033[2mEnter to send: " << endl;
        cout << "\033[0m";
        cin.getline(buffer, BUFFER_SIZE);
        string message(buffer);

        now_tm = getTime();

        // exit
        if (!strcmp(buffer, "exit"))
        {
            now_tm = getTime();
            closesocket(serverSocket);
            WSACleanup();
            logFile << "Server exited " << put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << endl;
            logFile.close();
            return 0;
        }
        else
            logFile << put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << " Message sent: " << message << endl;

        size_t pos = message.find("_caesar_:");
        if (pos != string::npos)
        {
            int shift = stoi(message.substr(pos + 9));              // Extract shift value
            message = CaesarEncrypt(message.substr(0, pos), shift); // Encrypt the message
            message += "_c_" + to_string(shift);
        }
        if (sendto(serverSocket, message.c_str(), message.size(), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            cerr << "sendto failed" << endl;
            logFile << "sendto failed" << put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }
        else
        {
            cout << "\033[32m";
            cout << put_time(&now_tm, "%H:%M:%S") << " message sent to server." << "\n\n";
            cout << "\033[0m";
        }
    }
}