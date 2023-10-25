#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>

int PORT = 5050;

int main()
{
    int sock;
    struct sockaddr_in address;
    std::vector<std::vector<int>> tabla;
    //bool playing = true;
    int x = 0;
    int y = 0;
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
    std::cout << buf << std::endl;
    return 0;
}