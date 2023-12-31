#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>
#include "../tokenize.hpp"
#include "../db.hpp"
#include <signal.h>

int PORT = 5050;
int conn_status = 0;
std::string name;
int sock2;
std::string pron;

void read_msg(int socket)
{
    int status = 0;
    while (true)
    {
        char *msg = (char *)calloc(1024, sizeof(char));
        status = recv(socket, msg, 1024, 0);
        if (conn_status == -1)
        {
            std::cout << "recv ended" << std::endl;
            return;
        }
        if (status == -1)
        {
            exit(0);
        }
        if (msg[0] != ' ' && msg[0] != '\0')
            std::cout << msg << std::endl;
        free(msg);
    }
}

std::string check_available_servers(int *sock)
{
    char *buf = (char *)calloc(1024, sizeof(char));
    std::string buff;
    struct sockaddr_in address;
    
    *sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*sock == -1)
    {
        std::cout << "ERROR" << std::endl;
    }
    std::cout << *sock << std::endl;
    
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    int conn = connect(*sock, (struct sockaddr*)&address, sizeof(address));
    send(*sock, "0", 1, 0);
    recv(*sock, buf, 1024, 0);
    buff = buf;
    free(buf);
    return buff;
}

std::string check_available_servers()
{
    char *buf = (char *)calloc(1024, sizeof(char));
    std::string buff;
    int sock;
    struct sockaddr_in address;
    
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
    close(sock);
    return buff;
}


void interr_func(int sig)
{
    std::string exit_str = "/exit";
    exit_str.append(1024 - exit_str.length(), '\0');
    send(sock2, exit_str.c_str(), 1024, 0);
    close(sock2);
    std::cout << "\nCatched!" << std::endl;
    exit(1);
}

int main()
{
    int sock;
    struct sockaddr_in address;
    struct sockaddr_in address2;
    std::string message;
    int conn;
    //std::vector<std::vector<int>> tabla;
    std::vector<std::string> a;
    //bool playing = true;
    signal(SIGINT, interr_func);
    std::cout << "What is your preferred username?: ";
    getline(std::cin, name);
    std::cout << "What is your preferred pronouns?: ";
    getline(std::cin, pron);
    std::cout << std::endl;
    name = std::format("{} ({})", name, pron);
    while (1)
    {
        char *buf = (char *)calloc(1024, sizeof(char));
        std::string buff;
        
        buff = check_available_servers(&sock);
        std::cout << "Chose a chat server!" << std::endl;
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
        system("clear");
        conn_status = 0;
        recv(sock, buf, 1024, 0);
        std::cout << buf << std::endl;
        free(buf);
        std::thread receive(read_msg, sock);
        receive.detach();
        send(sock, name.append(100 - name.length(), '\0').c_str(), 1024, 0);
        sock2 = sock;
        while(true)
        {
            std::getline(std::cin, message);
            if (message.compare("/exit") == 0)
            {
                send(sock, message.c_str(), message.length(), 0);
                message.clear();
                conn_status = -1;
                break;
            }
            else if (message.compare("/list servers") == 0) //TODO: make list command to also list users and so on
            {
                buff.clear();
                buff = check_available_servers();
                std::cout << "the available servers are: " << buff << std::endl;
                continue;
            }
            send(sock, message.c_str(), message.length(), 0);
            message.clear();
            std::cout << "\033[A\033[2K";
        }
    }
    return 0;
}
