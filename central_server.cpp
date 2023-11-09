#include "db.hpp"
#include "tokenize.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <ctime>
#include <format>
#include <chrono>

db database = db("Servers", {"Name", "Ip", "Port", "Api_Port"});
int PORT = 5050;
const std::string SERVER_IP = "0.0.0.0";

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
void log(T value, T value2)
{
    std::cout << "[" << get_time() << "] " << value << value2 << std::endl;
}

template<typename T, typename B>
void log(T value, B value2, T value3)
{
    std::cout << "[" << get_time() << "] " << value << value2 << value3 << std::endl;
}

template<typename T, typename B>
void log(T value, B value2)
{
    std::cout << "[" << get_time() << "] " << value << value2 << std::endl;
}

void manage_sv(int socket)
{
    char *buf = (char *)calloc(3, sizeof(char));
    char comp[1024];
    char sended[100];
    std::string buf2;
    std::vector<std::string> tokens;
    int index = 0;
    int status = 0;
    status = recv(socket, buf, 1, 0);
    if (status == -1)
        return;

    switch (ft_atoi(buf))
    {
        case 0:
            //handle client
            if (database.get_size() == 0)
                send(socket, "0", 1, 0);
            for (int x = 0; x < database.get_size(); x++)
            {
                if (x == 0)
                {
                    memcpy(comp, database.get_value(x)[0].c_str(), database.get_value(x)[0].length());
                    continue;
                }
                strcat(comp, ", ");
                strcat(comp, database.get_value(x)[0].c_str());
            }
            log(comp, database.get_size());
            send(socket, comp, 1024, 0);
            recv(socket, buf, 1, 0);
            buf2 = buf;
	        log("received: ", buf2);
            if (isNumber(buf2) && ft_atoi(buf) <= database.get_size() && ft_atoi(buf) >= 1)
            {
                buf2.clear();
                buf2 = std::format("{},{},{}", database.get_value(ft_atoi(buf) - 1)[1], database.get_value(ft_atoi(buf) - 1)[2], database.get_value(ft_atoi(buf) - 1)[3]);
                buf2.append(100 - buf2.length(), '\0');
                log("Sent server information ", buf2);
                send(socket, (char *)buf2.c_str(), 100, 0);
            }
            else
            {
                
                log("\x1B[91mError: The client didnt send a number or the number is invalid ",  buf2, "\033[0m\t\t");
            }
            memset(comp, 0, 1024);
            break;
        
        case 1:
            //handle server
            free(buf);
            buf = (char *)calloc(100, sizeof(char));
            status = recv(socket, buf, 100, 0);
            if (status == -1)
                return;
            buf2 = buf;
            log(buf2);
            tokens = tokenize(buf2, ',');
            if (tokens.size() != 4)
            {
                log("\x1B[91mError: The server didn't send enough information ", tokens.size(), "\033[0m\t\t");
                return;
            }
            if (!isNumber(tokens[2]))
            {
                log("\x1B[91mError: The port the server provided is not a number\033[0m\t\t");
                return;
            }
            index = database.add_value(tokens);
            log(database.get_value(0)[1]);
            while (1)
            {
                //keepalive
                status = send(socket, "1", 1, 0);
                if (status == -1)
                {
                    log(tokens[0], " has disconnected");
                    database.remove_value(index);
                    break;
                }
                buf = (char *)calloc(2, sizeof(char));
                status = recv(socket, buf, 1, 0);
                if (buf[0] != '1' || status == -1)
                {
                    log(tokens[0], " has disconnected");
                    database.remove_value(index);
                    free(buf);
                    break;
                }
                free(buf);
                log(tokens[0], " kept alive");
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            }
            break;


        default:
            break;
    }
}

void create_config()
{
    std::ofstream cfg("central_config.cfg");
    cfg << "//Central Server config\n";
    cfg << "//This changes the port the server listens to\n";
    cfg << "port:5050";
    cfg.close();
}

void load_config()
{
    std::ifstream infile("central_config.cfg");
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
                break;
            }
        }
        //linea.clear();
    }
}

int main()
{
    log("Starting server...");
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    log("Initializing socket...");
    if (sock == 0)
    {
        std::cout << "Se fallo al crear socket" << std::endl;
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
    log("listening" , std::format(" {}:{}", "localhost", PORT));
    
    if (std::filesystem::exists("central_config.cfg") == false)
    {
        create_config();
        log("Config created");
    }
    else
    {
        load_config();
        log("Config loaded");
    }
    while (true)
    {
        listen(sock, 5);
        int new_socket = accept(sock, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        log(new_socket);
        std::thread man_sv(manage_sv, new_socket);
        man_sv.detach();
    }
    return 0;
}
