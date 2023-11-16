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
#include <signal.h>

int PORT = 5052;


int main()
{
    int sock;
    struct sockaddr_in address;
    struct sockaddr_in address2;
    char *buf = (char *)calloc(101, sizeof(char));
    std::vector<std::string> a;
    std::string message = "get users";
    //bool playing = true;
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
    send(sock, message.append(100 - message.length(), '\0').c_str(), 100, 0);
    recv(sock, buf, 100, 0);
    std::cout << buf << std::endl;
}