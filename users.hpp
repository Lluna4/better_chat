#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

class user
{
    public:
        user() {}

        user(std::string uname, int socket)
            :uname_(uname), socket_(socket)
        {
            struct sockaddr_in buf;
            socklen_t len = sizeof(buf);
            if (getsockname(socket, (struct sockaddr *)&buf, &len) == 1)
                perror("getsockname error");
            test = buf;
            std::cout << "New user ip: " << test.sin_addr.s_addr << " New user port: " << test.sin_port << std::endl;
        }

        in_addr_t address()
        {
            return test.sin_addr.s_addr;
        }

        in_port_t port()
        {
            return test.sin_port;
        }

        std::string name()
        {
            return uname_;
        }

        int getsocket()
        {
            return socket_;
        }

    private:
        std::string uname_;
        int socket_;
        struct sockaddr_in test;
};