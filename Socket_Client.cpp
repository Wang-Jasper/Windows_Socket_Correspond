#include <iostream>
#include <winsock2.h>
// for the system time
#include <chrono>
#include <ctime>
#include <iomanip>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

// Caesar Cipher decryption function
string CaesarDecrypt(const string &text, int shift)
{
    string result = text;
    for (char &c : result)
    {
        if (isalpha(c))
        {
            char base = islower(c) ? 'a' : 'A';
            c = (c - base - shift + 26) % 26 + base;
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
    SOCKET clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[BUFFER_SIZE];
    int clientAddrSize = sizeof(clientAddr);
    tm now_tm;

    ofstream logFile("socket_log.txt", ios::app); // Append mode
    if (!logFile.is_open())
    {
        cerr << "Failed to open log file" << endl;
        return 1;
    }

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        now_tm = getTime();
        cerr << "WSAStartup failed" << endl;
        logFile << "WSAStartup failed " << put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << endl;
        return 1;
    }

    // create socket
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        now_tm = getTime();
        cerr << "Socket creation failed" << endl;
        logFile << "Client_Socket creation failed " << put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << endl;
        WSACleanup();
        return 1;
    }
    else
    {
        now_tm = getTime();
        logFile << "Client_Socket Created on " << put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << endl;
    }

    // server adress
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // accept any ip and port
    serverAddr.sin_port = htons(PORT);

    // bind socket
    if (bind(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cerr << "Bind failed" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    cout << "\033[33m" << "\033[2m";
    cout << "Application is running and waiting for messages..." << endl;

    char *ipTmp;
    int portTmp = 0;
    // receive
    while (true)
    {
        cout << "\033[0m";
        int recvLen = recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientAddrSize);
        now_tm = getTime();
        if (recvLen == SOCKET_ERROR)
        {

            cerr << "recvfrom failed" << endl;
            logFile << "recvfrom failed" << put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        buffer[recvLen] = '\0'; // Null-terminate the received data
        // Get client IP address and port
        char *clientIP = inet_ntoa(clientAddr.sin_addr);
        int clientPort = ntohs(clientAddr.sin_port);

        // Caesar Decrypt
        string message(buffer), enMessage = "";
        size_t pos = message.find("_c_");
        if (pos != string::npos)
        {
            int shift = stoi(message.substr(pos + 3));
            message = message.substr(0, pos);
            enMessage = CaesarDecrypt(message, shift);
        }

        if (clientPort != portTmp)
        {
            if (portTmp != 0)
                cout << "\n________________________________________\n";
            cout << "\033[32m";
            cout << "Received message from: ";
            cout << "\033[1m" << "\033[34m" << "\033[3m";
            cout << clientIP << " : " << clientPort << endl;
            cout << "\033[0m";

            ipTmp = clientIP;
            portTmp = clientPort;
        }
        logFile << "Received message from " << clientIP << ":" << clientPort << "\t" << put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << endl;
        cout << "   " << message << "\t\t\033[2m" << put_time(&now_tm, "%H:%M:%S") << endl;

        if (enMessage != "")
            cout << "   \033[31mCipher Decrypted: " << enMessage << endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}