#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <string>
#include <string.h>
#include <cstring>
#include <cerrno>

struct address
{
	std::string ip;
	int port;
};

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
			return ;
		buff = buf;
		formatted_string = std::format("{}: {}", uname, buff);
		send(sock, formatted_string.c_str(), 1024, 0);
		memset(buf, 0, 1025);
	}
	close(sock);
}

void listen_th(int sock, sockaddr_in addr)
{
	int addrlen = sizeof(addr);
	while (1)
	{
		listen(sock, 32);
		int new_sock = accept(sock, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
		std::thread sv(server, new_sock);
		sv.detach();
	}
}

address start_msg_sv(int port)
{
	struct address ret;
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == 0)
		return ret;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
	addr.sin_port = htons(port) + 1;

	if (bind(sock, (struct sockaddr *)&addr, (socklen_t)addrlen) < 0) 
	{
		std::cerr << "Bind failed: " << strerror(errno) << std::endl;
		exit(-1);
	}
	std::thread listen_thread(listen_th, sock, addr);
	listen_thread.detach();
	
	ret.port = htons(port) + 1;
	ret.ip = "127.0.0.1";
	return ret;
}
