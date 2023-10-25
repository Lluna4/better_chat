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

db database = db("Servers", {"Name", "Ip", "Port"});
int PORT = 5050;
const std::string SERVER_IP = "0.0.0.0";

char* ft_strjoin(char const* s1, char const* s2)

{
	char* ret;
	int		n;

	n = -1;
	if (*s1 == '\0' && *s2 == '\0')
		return (strdup(""));
	ret = (char *)calloc(strlen(s1) + strlen(s2) + 1, sizeof(char));
	if (!ret)
		return (0);
	while (*s1 != '\0')
	{
		n++;
		ret[n] = *s1;
		s1++;
	}
	while (*s2 != '\0')
	{
		n++;
		ret[n] = *s2;
		s2++;
	}
	return (ret);
}

char* ft_strjoin(char const* s1, char const* s2, char const *s3)

{
	char* ret;
	int		n;

	n = -1;
	if (*s1 == '\0' && *s2 == '\0')
		return (strdup(""));
	ret = (char *)calloc(1024, sizeof(char));
	if (!ret)
		return (0);
	while (*s1 != '\0')
	{
		n++;
		ret[n] = *s1;
		s1++;
	}
	while (*s2 != '\0')
	{
		n++;
		ret[n] = *s2;
		s2++;
	}
	while (*s3 != '\0')
	{
		n++;
		ret[n] = *s3;
		s3++;
	}
	return (ret);
}

std::string get_time()
{
	time_t tiempo;
	time(&tiempo);
	struct tm* a = (tm*)malloc(1 * sizeof(tm));
	localtime_r(&tiempo, a);
	char* min = strdup(ft_itoa(a->tm_min));
    char* sec = strdup(ft_itoa(a->tm_sec));
	if (strlen(min) == 1)
		min = ft_strjoin("0", min);
    if (strlen(sec) == 1)
		sec = ft_strjoin("0", sec);
	return std::format("{}:{}:{}", a->tm_hour, min, sec);
}

void manage_sv(int socket)
{
    char *buf = (char *)calloc(3, sizeof(char));
    char *comp;
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
            comp = (char *)calloc(1024, sizeof(char));
            for (int x = 0; x < database.get_size(); x++)
            {
                if (x == 0)
                {
                    memcpy(comp, database.get_value(x)[0].c_str(), database.get_value(x)[0].length());
                    continue;
                }
                comp = ft_strjoin(comp, ",", database.get_value(x)[0].c_str());
            }
            std::cout << "[" << get_time() << "] "<< comp << database.get_size() << std::endl;
            send(socket, comp, 1024, 0);
            recv(socket, buf, 1, 0);
            buf2 = buf;
            if (isNumber(buf2) && ft_atoi(buf) <= database.get_size())
            {
                free(comp);
                buf = ft_strjoin(database.get_value(ft_atoi(buf))[1].c_str(), ", ", database.get_value(ft_atoi(buf))[2].c_str());
                comp = (char *)calloc(100, sizeof(char));
                strncpy(comp, buf, strlen(buf));
                send(socket, comp, 100, 0);
            }
            else
            {
                std::cout << "[" << get_time() << "] "<< "\x1B[91mError: The client didnt send a number or the number is invalid " << buf2 <<  "\033[0m\t\t" << std::endl;
            }
            break;
        
        case 1:
            //handle server
            free(buf);
            buf = (char *)calloc(100, sizeof(char));
            status = recv(socket, buf, 100, 0);
            if (status == -1)
                return;
            buf2 = buf;
            std::cout << "[" << get_time() << "] "<< buf2 << std::endl;
            tokens = tokenize(buf2, ',');
            if (tokens.size() != 3)
            {
                std::cout << "[" << get_time() << "] "<< "\x1B[91mError: The server didn't send enough information " << tokens.size() <<  "\033[0m\t\t" << std::endl;
                return;
            }
            if (!isNumber(tokens[2]))
            {
                std::cout << "[" << get_time() << "] "<< "\x1B[91mError: The port the server provided is not a number\033[0m\t\t" << std::endl;
                return;
            }
            index = database.add_value(tokens);
            std::cout << "[" << get_time() << "] "<< database.get_value(0)[1] << std::endl;
            while (1)
            {
                //keepalive
                status = send(socket, "1", 1, 0);
                if (status == -1)
                {
                    std::cout << "[" << get_time() << "] "<< tokens[0] << " has disconnected"<< std::endl;
                    database.remove_value(index);
                    break;
                }
                free(buf);
                buf = (char *)calloc(2, sizeof(char));
                status = recv(socket, buf, 1, 0);
                if (buf[0] != '1' || status == -1)
                {
                    std::cout << "[" << get_time() << "] "<< tokens[0] << " has disconnected"<< std::endl;
                    database.remove_value(index);
                    break;
                }
                memset(buf, 0, 1);
                std::cout << "[" << get_time() << "] "<< tokens[0] << " kept alive"<< std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            }
            break;


        default:
            break;
    }
}

int main()
{
    std::cout << "[" << get_time() << "] " << "Starting server..." << std::endl;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "[" << get_time() << "] "<< "Initializing socket..." << std::endl;
    if (sock == 0)
    {
        std::cout << "Se fallo al crear socket" << std::endl;
        return -1;
    }
    address.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP.c_str(), &address.sin_addr);
    address.sin_port = htons(PORT);
    std::cout << "[" << get_time() << "] "<< "Binding..." << std::endl;
    if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        std::cout << "[" << get_time() << "] "<< "Bind failed" << std::endl;
    }
    std::cout << "[" << get_time() << "] "<< "listening" << " localhost:"<< PORT << std::endl;
    while (true)
    {
        listen(sock, 5);
        int new_socket = accept(sock, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        std::cout << new_socket << std::endl;
        std::thread man_sv(manage_sv, new_socket);
        man_sv.detach();
    }
    return 0;
}