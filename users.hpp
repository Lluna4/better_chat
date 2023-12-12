#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>

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

    private:
        std::string uname_;
        int socket_;
        struct sockaddr_in test;
};