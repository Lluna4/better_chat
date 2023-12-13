#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <fstream>
#include "tokenize.hpp"
#include "db.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


std::vector<std::string> mess;
int PORT = 5050;
int API_PORT = 5052;
int new_render = 0;
int message_sent = 0;
int first = 1;
std::string bottom_text = "Enter message: ";
std::string uname;
int sv = 0;

void clear_messages()
{
    mess.clear();
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
    
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    int conn = connect(sock, (struct sockaddr*)&address, sizeof(address));
    send(sock, "0", 1, 0);
    recv(sock, buf, 1024, 0);
    buff = buf;
    free(buf);
    close(sock);
    return std::format("The available servers are: {}", buff);
}

void free_unseen_messages(int maxy)
{
    int index = 0;
    if (maxy < mess.size() && maxy > 1)
    {
        while (maxy < mess.size())
        {
            mess.erase(mess.begin() + index);
            index++;
        }
    }
}

void connection()
{
    int sock;
    int conn;
    struct sockaddr_in address;
    struct sockaddr_in address2;
    char *buf = (char *)calloc(101, sizeof(char));
    std::string buff;
    std::vector<std::string> a;
    int status = 0;
    std::string sv_name;
    
    buff = check_available_servers(&sock);
    mess.push_back("Select a server!");
    if (buff.find(',') != std::string::npos)
    {
        a = tokenize(buff, ',');
        for (size_t x = 0; x < a.size(); x++)
        {
            mess.push_back(std::format("{}) {}\n", (x + 1), a[x].c_str()));
        }
    }
    else
    {
        mess.push_back(std::format("1) {}\n", buff));
        a.push_back(buff);
    }
    new_render = 1;
    while (1)
    {
        if (message_sent == 1)
        {
            send(sock, (char *)mess[mess.size() - 1].c_str(), 1, 0);
            sv_name = a[atoi(mess[mess.size() - 1].c_str()) - 1];
            mess.pop_back();
            break;
        }
    }
    recv(sock, buf, 100, 0);
    buff = buf;
    a = tokenize(buf, ',');
    if (a[0].compare("0.0.0.0") == 0)
    {
        a[0] = "127.0.0.1";
    }
    API_PORT = atoi(a[2].c_str());
    close(sock);
    sock = socket(AF_INET, SOCK_STREAM, 0);
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
        return;
    }
    clear_messages();
    mess.push_back(std::format("You joined {}", sv_name));
    recv(sock, buf, 1024, 0);
    buff = buf;
    mess.push_back(buff);
    new_render = true;
    send(sock, uname.append(100 - uname.length(), '\0').c_str(), 100, 0);

    //recv_message
    memset(buf, 0, 1024);
    sv = sock;
    while (1)
    {
        status = recv(sock, buf, 1024, 0);
        if (status == -1)
        {
            free(buf);
            break;
        }
        buff = buf;
        //buff = std::format("+------------------+\n|{0:<18}|\n+------------------+\n|{1:<18}|\n+------------------+", "Luna", buff);
        mess.push_back(buff);
        new_render = true;
        memset(buf, 0, 1024);
    }
}

void render_pad(WINDOW *pad)
{
    int max_y = 0, max_x = 0;
    int pad_pos = 0;

    //attron(COLOR_PAIR(2));
    while (1)
    {
        if (new_render && first == 0)
        {
            getmaxyx(stdscr, max_y, max_x);
            werase(pad);
            for (int x = 0; x < mess.size(); x++)
            {
                if (mess[x].starts_with("The"))
                {
                    wattron(pad, COLOR_PAIR(2));
                    mvwprintw(pad, x, 0, mess[x].c_str());
                    wattroff(pad, COLOR_PAIR(2));
                    continue;
                }
                else if (mess[x].contains("has disconnected"))
                {
                    wattron(pad, COLOR_PAIR(3));
                    mvwprintw(pad, x, 0, mess[x].c_str());
                    wattroff(pad, COLOR_PAIR(3));
                    continue;
                }
                mvwprintw(pad, x, 0, mess[x].c_str());
            }
            pad_pos = mess.size() > max_y - 2 ? mess.size() - max_y + 2 : 0;
            prefresh(pad, pad_pos, 0, 0, 0, max_y - 2, max_x - 1);
            new_render = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        free_unseen_messages(max_y);
    }
}

std::string get_users()
{
    int sock;
    struct sockaddr_in address;
    char *buf = (char *)calloc(101, sizeof(char));
    std::vector<std::string> a;
    std::string message = "get users";
    //bool playing = true;
    std::string buff;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_family = AF_INET;
    address.sin_port = htons(API_PORT);
    int conn = connect(sock, (struct sockaddr*)&address, sizeof(address));
    send(sock, message.append(100 - message.length(), '\0').c_str(), 100, 0);
    recv(sock, buf, 100, 0);
    
    buff = buf;
    return std::format("The users connected now are: {}", buff);
}

void command(std::string com)
{
    std::vector<std::string> tokens = tokenize(com);
    std::string ret;

    if(tokens[0].compare("list") == 0)
    {
        if (tokens.size() < 2)
            return;
        else if (tokens[1].compare("servers") == 0)
        {
            ret = check_available_servers();
            mess.push_back(ret);
            new_render = true;
        }
        else if (tokens[1].compare("users") == 0)
        {
            ret = get_users();
            mess.push_back(ret);
            new_render = true;
        }
        
    }

}

int main() 
{
    std::thread conn_th(connection);
    conn_th.detach();
    initscr();
    cbreak();
    curs_set(0);
    if (has_colors() == true)
    {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(3, COLOR_RED, COLOR_BLACK);
        init_color(10, 227, 216, 2);
        init_pair(4, 10, COLOR_BLACK);
        mess.push_back(std::format("Terminal has {} colors", COLORS));
    }

    char *buf = (char *)calloc(1024, sizeof(char));
    std::string buff;
    int max_y = 0, max_x = 0;
    int pad_pos = 0;
    WINDOW *pad = newpad(1000, COLS);
    std::thread render_pad_th(render_pad, pad);
    render_pad_th.detach();

    while (true)
    {
        attron(COLOR_PAIR(1));
        getmaxyx(stdscr, max_y, max_x);
        move(max_y - 1, 0);
        clrtoeol();
        if (first == 1)
        {
            bottom_text = "Enter username: ";
            mvprintw(max_y - 1, 0, bottom_text.c_str());
            mvgetstr(max_y - 1, bottom_text.length(), buf);
            uname = buf;
            first = 0;
            bottom_text = "Select server: ";
            memset(buf, 0, 1024);
            clrtoeol();
            mvprintw(max_y - 1, 0, bottom_text.c_str());
            mvgetstr(max_y - 1, bottom_text.length(), buf);
            buff = buf;
            mess.push_back(buff);
            first = 0;
            new_render = true;
            message_sent = 1;
            memset(buf, 0, 1024);
            continue;
        }
        mvprintw(max_y - 1, 0, "Enter message: ");
        mvgetstr(max_y - 1, strlen("Enter message: "), buf);

        if (buf[0] == '\0')
            continue;
        
        buff = buf;
        if (buff[0] == '/')
        {
            buff.erase(0, 1);
            command(buff);
            continue;
        }
        send(sv, buff.append(1024 - buff.length(), '\0').c_str(), 1024, 0);
        memset(buf, 0, 1024);
    }
    delwin(pad);
    endwin();
}