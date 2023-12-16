#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <format>
#include <string.h>
#include <thread>

void manage_sv(int socket)
{
    int status = 0;
    std::string uname;
    std::string formatted_string;
    std::string buff;
    char *buf = (char *)calloc(1025, sizeof(char));

    recv(socket, buf, 1024, 0);
    uname = buf;
    memset(buf, 0, 1024);
    while(true)
    {
        status = recv(socket, buf, 1024, 0);
        if (status == -1 || strcmp(buf, "/exit") == 0)
        {
            break;
        }
        if (buf[0] == '\0')
            break;

        buff = buf;
        formatted_string = std::format("{}:  {}", uname, buf);
        send(socket, formatted_string.c_str(), 1024, 0);
        memset(buf, 0, 1024);
        formatted_string.clear();
    }
    free(buf);
    close(socket);
}

void listen_th(int port, int sock, struct sockaddr_in address)
{
    int addrlen = sizeof(address);
    listen(sock, 32);
    
    int new_socket = accept(sock, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    std::thread man_sv(manage_sv, new_socket);
    man_sv.detach();
    new_socket = accept(sock, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    std::thread man_svv(manage_sv, new_socket);
    man_svv.detach();
}

void start_msg_sv(int port, int *sockk)
{
    struct sockaddr_in address;
    struct sockaddr_in address2;
    int addrlen = sizeof(address);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == 0)
    {
        return ;
    }
    address.sin_family = AF_INET;
    address2.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);
    address2.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);
    address2.sin_port = htons(port);
    if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        return;
    }
    std::thread listen_thread(listen_th, port, sock, address);
    listen_thread.detach();
    connect(*sockk, (struct sockaddr*)&address2, addrlen);
}