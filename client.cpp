#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>
#include "tokenize.hpp"
#include "db.hpp"

int PORT = 5050;

int main()
{
    int sock;
    struct sockaddr_in address;
    struct sockaddr_in address2;
    std::vector<std::vector<int>> tabla;
    std::vector<std::string> a;
    //bool playing = true;
    char *buf = (char *)calloc(1024, sizeof(char));
    std::string buff;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        std::cout << "ERROR" << std::endl;
    }
    std::cout << sock << std::endl;
    
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    int conn = connect(sock, (struct sockaddr*)&address, sizeof(address));
    send(sock, "0", 1, 0);
    recv(sock, buf, 1024, 0);
    buff = buf;
    free(buf);
    if (buff.find(',') != std::string::npos)
    {
        a = tokenize(buff, ',');
        for (size_t x = 0; x < a.size(); x++)
        {
            printf("%lu) %s\n", (x + 1), a[x].c_str());
        }
    }
    else
    {
        std::cout << "1) " << buff << std::endl;
    }
    buff.clear();
    std::cin >> buff;
    if (isNumber(buff) == true)
    {
        send(sock, (char *)buff.c_str(), 1, 0);
        buf = (char *)calloc(100 + 1, sizeof(char));
        recv(sock, buf, 100, 0);
    }
    close(sock);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    a.clear();
    a = tokenize(buf, ',');
    if (a[0].compare("0.0.0.0") == 0)
    {
        a[0] = "127.0.0.1";
    }
    address2.sin_addr.s_addr = inet_addr(a[0].c_str());
    address2.sin_family = AF_INET;
    address2.sin_port = htons(atoi(a[1].c_str()));
    conn = connect(sock, (struct sockaddr*)&address2, sizeof(address2));
    free(buf);
    buf = (char *)calloc(1024 + 1, sizeof(char));
    if (conn == -1)
    {
        free(buf);
        std::cout << "\x1B[91mNo se ha podido conectar con el servidor de chat\033[0m\t\t" << std::endl;
        return -1;
    }
    recv(sock, buf, 1024, 0);
    std::cout << buf << std::endl;
    free(buf);

    return 0;
}
