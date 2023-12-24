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
#include <condition_variable>
#include <poll.h>
#include <fcntl.h>

struct address
{
	std::string ip;
	int port;
};

std::vector<int> clients;
std::vector<std::jthread> threads;


void server(std::stop_token stoken, int sock)
{
	struct pollfd pfds[1];
    int status = 0;
	std::string uname;
	std::string formatted_string;
	std::string buff;
	int num_events = 0;
	char *buf = (char *)calloc(1025, sizeof(char));
	pfds[0].fd = sock;
	pfds[0].events = POLLIN;

	recv(sock, buf, 1024, 0);
	uname = buf;
	memset(buf, 0, 1024);
	while (1)
	{
		if (stoken.stop_requested())
		{
			close(sock);
			free(buf);
			return;
		}
		num_events = poll(pfds, 1, 1000);
		if (num_events == 0)
			continue;
		read(sock, buf, 1024);
		if (status == -1)
			break;
		if (buf[0] == '\0')
            break;
		buff = buf;
		formatted_string = std::format("{}: {}", uname, buff);
		for (int i = 0; i < clients.size();i++) 
			send(clients[i], formatted_string.c_str(), 1024, 0);
		memset(buf, 0, 1025);
	}
	close(sock);
	free(buf);
}

void listen_th(std::stop_token stoken, int sock, sockaddr_in addr)
{
	socklen_t addrlen = sizeof(addr);
    while (1)
    {
		if (stoken.stop_requested())
		{
			close(sock);
			return ;
		}
        listen(sock, 32);
        int new_sock = accept(sock, nullptr, nullptr);
        if (new_sock < 0) 
        {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        clients.push_back(new_sock);
		threads.emplace_back(server, new_sock);
    }
}

address start_msg_sv(int port)
{
	struct address ret;
	clients.clear();
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(sock, F_SETFL, O_NONBLOCK);
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
	threads.emplace_back(listen_th, sock, addr);
	ret.port = port;
	ret.ip = "127.0.0.1";
	return ret;
}
