#include "db.hpp"
#include "tokenize.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <ctime>
#include <format>
#include <filesystem>

int PORT = 5052;
int conn_PORT = 5050;
const std::string SERVER_IP = "0.0.0.0";
const char *name = "Lunasv2";

std::string get_time()
{
	time_t tiempo;
	time(&tiempo);
	struct tm* a = (tm*)malloc(1 * sizeof(tm));
	localtime_r(&tiempo, a);
    std::string min = ft_itoa(a->tm_min);
    std::string sec = ft_itoa(a->tm_sec);
	if (min.length() == 1)
		min = std::format("{}{}", "0", min);
    if (sec.length() == 1)
		sec = std::format("{}{}", "0", sec);
    std::string ret = std::format("{}:{}:{}", a->tm_hour, min, sec);
    free(a);
	return ret;
}

template<typename T>
void log(T value)
{
    std::cout << "[" << get_time() << "] " << value << std::endl;
}

template<typename T>
void log(T value, T value2)
{
    std::cout << "[" << get_time() << "] " << value << value2 << std::endl;
}

template<typename T, typename B>
void log(T value, B value2)
{
    std::cout << "[" << get_time() << "] " << value << value2 << std::endl;
}

void keepalive(int socket)
{
    char *buf = (char *)calloc(2, sizeof(char));
    while (1)
    {
        recv(socket, buf, 1, 0);
        send(socket, buf, 1, 0);
    }
}


void get_servers()
{
    int sock;
    struct sockaddr_in address;
    std::vector<std::vector<int>> tabla;
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
    address.sin_port = htons(conn_PORT);
    int conn = connect(sock, (struct sockaddr*)&address, sizeof(address));
    send(sock, "1", 1, 0);
    log(conn);
    std::string sended = std::format("{},{},{}", name, SERVER_IP, ft_itoa(PORT));
    log(sended);
    char *comp = (char *)calloc(100, sizeof(char));
    strncpy(comp, sended.c_str(), sended.length());
    send(sock, comp, 100, 0);
    std::thread keepaliv(keepalive, sock);
    keepaliv.detach();
}

void create_config()
{
    std::ofstream cfg("sv.cfg");
    cfg << "//Central Server config\n";
    cfg << "//This changes the port the server listens to\n";
    cfg << "port:5051";
    cfg << "//This changes the name of the server for the client";
    cfg << "name:Test_sv";
    cfg.close();
}

void load_config()
{
    std::ifstream infile("sv.cfg");
    std::string linea;
    while (std::getline(infile, linea))
    {
        //std::cout << linea << std::endl;
        if (linea.starts_with("//") == false)
        {
            std::vector<std::string> values = tokenize(linea, ':');
            if (values[0].compare("port") == 0)
            {
                PORT = atoi(values[1].c_str());
            }
            else if (values[0].compare("name") == 0)
            {
                name = (char *)values[1].c_str();    
            }
        }
        //linea.clear();
    }
}

int main()
{
    std::cout << "[" << get_time() << "] " << "Starting server..." << std::endl;
    log("Starting server...");
    struct sockaddr_in address;
    //int addrlen = sizeof(address);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (std::filesystem::exists("sv.cfg") == false)
    {
        create_config();
        log("Config created");
    }
    else
    {
        load_config();
        log("Config loaded");
        log("name:", name);
        log("port:", PORT);
    }
    log("Initializing socket...");
    if (sock == 0)
    {
        log("Failed to create socket");
        return -1;
    }
    address.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP.c_str(), &address.sin_addr);
    address.sin_port = htons(PORT);
    std::cout << "[" << get_time() << "] "<< "Binding..." << std::endl;
    log("Binding..");
    if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        log("Bind failed");
    }
    log("listening", std::format(" {}:{}", "localhost", PORT));
    log("Sending information to the central server");
    get_servers();
    log("Information sent");
    while (true)
    {
        listen(sock, 32);
       // int new_socket = accept(sock, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        /*std::cout << new_socket << std::endl;
        std::thread man_sv(manage_sv, new_socket);
        man_sv.detach();*/
    }
    return 0;
}
