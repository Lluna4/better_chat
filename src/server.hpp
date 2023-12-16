#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <format>
#include <string.h>

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
        formatted_string = std::format("{}: {}", uname, buf);
        send(socket, formatted_string.c_str(), 1024, 0);
        memset(buf, 0, 1024);
        formatted_string.clear();
    }
    free(buf);
    close(socket);
}

void start_msg_sv(int port)
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == 0)
    {
        return ;
    }
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);
    address.sin_port = htons(port);
    if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        return;
    }
    listen(sock, 32);
    int new_socket = accept(sock, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    manage_sv(new_socket);
}