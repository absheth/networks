
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT_NO 8080

typedef struct
{
    std::string Dst;
    std::string nxtHop;
    int cost;
    u_short ttl;
} route_entry;

int main(int argc, char *argv[])
{
    std::cout << "***UDP***" << std::endl;
    struct sockaddr_in server, client;
    socklen_t len;
    int server_fd;

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(server_fd == -1)
    {
        std::perror("Socket creation failed");
        return -1;
    }

    //Clearing the socket structure
    memset(&server, 0, sizeof(struct sockaddr_in));

    //Define socket parameters
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NO);

    //Binding the socket
    if(bind(server_fd, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) == -1)
    {
        std::perror("Socket bind failed");
        close(server_fd);
        return -1;
    }

    int char_read, char_write;
    socklen_t client_len = sizeof(client);
    char request_buffer[sizeof(route_entry)*3];

    //Clear the input buffer and get the client request
    memset(request_buffer, 0, sizeof(request_buffer));

    char_read = recvfrom(server_fd, request_buffer, (sizeof(route_entry)*3), 0, (struct sockaddr*) &client, &client_len);
    if(char_read < 0)
    {
        std::perror("Read failed2");
    }

    route_entry route_table[3];

    memcpy(&route_table, &request_buffer, (sizeof(route_entry)*3));
    std::perror("Read error");

    for(int i=0; i < 3; i++)
    {
        std::cout << route_table[i].Dst << std::endl;
    }
}
