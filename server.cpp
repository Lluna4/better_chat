#include "db.hpp"
#include "tokenize.hpp"
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <ctime>
#include <format>
#include <filesystem>
#include "users.hpp"

int PORT = 5052;
int conn_PORT = 5050;
int API_PORT = 5053;
const std::string SERVER_IP = "0.0.0.0";
std::string motd = "Hola! Si ves esto es que todo ha ido genial!";
const char *name = "Lunasv2";
std::vector<user> usuarios;

std::string get_time()
{
	time_t tiempo;
	time(&tiempo);
	struct tm* a = (tm*)malloc(1 * sizeof(tm));
	localtime_r(&tiempo, a);
    std::string h = ft_itoa(a->tm_hour);
    std::string min = ft_itoa(a->tm_min);
    std::string sec = ft_itoa(a->tm_sec);
	if (h.length() == 1)
		h = std::format("{}{}", "0", h);
	if (min.length() == 1)
		min = std::format("{}{}", "0", min);
    if (sec.length() == 1)
		sec = std::format("{}{}", "0", sec);
    std::string ret = std::format("{}:{}:{}", h, min, sec);
    free(a);
	return ret;
}

template<typename T>
void log(T value)
{
    std::cout << "[" << get_time() << "] " << value << std::endl;
}

template<typename T>
void log_err(T value)
{
    std::cout <<"\x1B[91m" << "[" << get_time() << "] " << value << "\033[0m\t\t" << std::endl;
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

void manage_api(int socket)
{
    char *buf = (char *)calloc(101, sizeof(char));
    std::vector<std::string> tokens;
    std::string ret = "0";
    recv(socket, buf, 100, 0);
    tokens = tokenize(buf, ' ');
    free(buf);
    if (tokens.size() < 2)
    {
        send(socket, ret.append(100 - ret.length(), '\0').c_str(), 100, 0);
        return;
    }
    if (tokens[0].compare("get") == 0)
    {
        if (tokens[1].compare("users") == 0)
        {
            buf = (char *)calloc(101, sizeof(char));
            ret.clear();
            log("Getting users...");
            for (int x = 0; x < usuarios.size(); x++)
            {
                if (x == 0)
                {
                    memcpy(buf, usuarios[x].name().c_str(), usuarios[x].name().length());
                    continue;
                }
                strcat(buf, ", ");
                strcat(buf, usuarios[x].name().c_str());
            }
        }
    }
    log("api sent:", buf);
    send(socket, buf, 100, 0);
    free(buf);
    close(socket);
}

void not_an_api()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    log("Initializing api server socket...");
    if (sock == 0)
    {
        log("Failed to create socket");
        return ;
    }
    address.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP.c_str(), &address.sin_addr);
    address.sin_port = htons(API_PORT);
    log("Binding api server...");
    if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        log("Bind failed");
    }
    log("Api server listening", std::format(" {}:{}", "localhost", API_PORT));
    while (true)
    {
        listen(sock, 32);
        int new_socket = accept(sock, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        log("New api client: ", new_socket);
        std::thread man_sv(manage_api, new_socket);
        man_sv.detach();
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
    std::string sended = std::format("{},{},{},{}", name, SERVER_IP, ft_itoa(PORT), ft_itoa(API_PORT));
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
    cfg << "//This changes the port the chat server listens to\n";
    cfg << "port:5051\n";
    cfg << "//This changes the port the api server listens to\n";
    cfg << "api_port:5052\n";
    cfg << "//This changes the name of the server for the client\n";
    cfg << "name:Test_sv\n";
    cfg << "//This changes the motd of the server (message that sends first)\n";
    cfg << "motd:Hola! Si ves esto es que todo ha ido genial!\n";
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
            else if (values[0].compare("api_port") == 0)
            {
                API_PORT = atoi(values[1].c_str());
            }
            else if (values[0].compare("name") == 0)
            {
                name = (char *)values[1].c_str();    
            }
            else if (values[0].compare("motd") == 0)
            {
                motd = values[1];    
            }
        }
        //linea.clear();
    }
}

/*
void command(std::string a, int socket)
{
    std::vector<std::string> token = tokenize(a);

    if (token.size() > 2)
    {
        if (token[0].compare("/msg") == 0)
        {
            for (int i = 0; i < unames.size(); i++)
            {
                if (token[1].compare(unames[i]) == 0)
                {
                    send(socket, "Se ha encontrado a alguien", 1024, 0);
                }
            }
        }
    }
}*/

void manage_sv(int socket)
{
    int status = 0;
    std::string uname;
    std::string formatted_string;
    std::string buff;
    char *buf = (char *)calloc(1025, sizeof(char));
    log("Client connected");
    motd.append(1024 - motd.length(), '\0');
    send(socket, motd.c_str(), 1024, 0);
    log("Motd sent: ", motd);

    recv(socket, buf, 1024, 0);
    uname = buf;
    memset(buf, 0, 1024);
    log("Username: ", uname);
    usuarios.push_back(user(uname, socket));
    for (size_t x = 0; x < usuarios.size(); x++)
    {
        formatted_string = std::format("{} has connected", uname);
        send(usuarios[x].getsocket(), formatted_string.c_str(), 1024, 0);
    }
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
        for (size_t x = 0; x < usuarios.size(); x++)
        {
            formatted_string = std::format("{}: {}", uname, buf);
            send(usuarios[x].getsocket(), formatted_string.c_str(), 1024, 0);
        }
        log("message received.");
        memset(buf, 0, 1024);
        formatted_string.clear();
    }
    log_err(std::format("{}{}{}", "Client: ", uname, " has disconnected"));
    for (unsigned int i = 0; i < usuarios.size(); i++)
    {
        if (usuarios[i].getsocket() == socket)
        {
            usuarios.erase(usuarios.begin() + i);
            break;
        }
    }
    if (usuarios.size() > 0)
    {
        for (size_t x = 0; x < usuarios.size(); x++)
        {
            formatted_string = std::format("{} has disconnected", uname);
            send(usuarios[x].getsocket(), formatted_string.c_str(), 1024, 0);
        }
    }
    free(buf);
    close(socket);
}


int main()
{
    std::cout << "[" << get_time() << "] " << "Starting server..." << std::endl;
    log("Starting server...");
    struct sockaddr_in address;
    int addrlen = sizeof(address);
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
        log("api port:", API_PORT);
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
    log("Binding...");
    if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        log("Bind failed");
    }
    log("listening", std::format(" {}:{}", "localhost", PORT));
    log("Sending information to the central server");
    get_servers();
    log("Information sent");
    std::thread api_th(not_an_api);
    api_th.detach();
    log("Api server started");
    while (true)
    {
        listen(sock, 32);
        int new_socket = accept(sock, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        log("New client: ", new_socket);
        std::thread man_sv(manage_sv, new_socket);
        man_sv.detach();
    }
    return 0;
}
