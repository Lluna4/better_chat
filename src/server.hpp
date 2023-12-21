#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <string>
#include <string.h>
#include <cstring>
#include <cerrno>
#include <vector>
#include <exception>

struct address
{
	std::string ip;
	int port;
};

std::vector<int> clients;

void server(int sock)
{
    int status = 0;
	std::string uname;
	std::string formatted_string;
	std::string buff;
	char *buf = (char *)calloc(1025, sizeof(char));

	recv(sock, buf, 1024, 0);
	uname = buf;
	memset(buf, 0, 1024);
	while (1)
	{
		status = recv(sock, buf, 1024, 0);
		if (status == -1)
			break;
		buff = buf;
		formatted_string = std::format("{}: {}", uname, buff);
		for (int i = 0; i < clients.size();i++) 
			send(clients[i], formatted_string.c_str(), 1024, 0);
		memset(buf, 0, 1025);
	}
	close(sock);
}

void listen_th(int sock, sockaddr_in addr)
{
	socklen_t addrlen = sizeof(addr);
    while (1)
    {
        listen(sock, 32);
        int new_sock = accept(sock, nullptr, nullptr);
        if (new_sock < 0) 
        {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
            continue;
        }
        clients.push_back(new_sock);
        std::thread sv(server, new_sock);
        sv.detach();
    }
}

address start_msg_sv(int port)
{
	struct address ret;
	clients.clear();
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == 0)
		return ret;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
	addr.sin_port = port;

	if (bind(sock, (struct sockaddr *)&addr, (socklen_t)addrlen) < 0) 
	{
		std::cerr << "Bind failed: " << strerror(errno) << std::endl;
		exit(-1);
	}
	std::thread listen_thread(listen_th, sock, addr);
	listen_thread.detach();
	
	ret.port = port;
	ret.ip = "127.0.0.1";
	return ret;
}
