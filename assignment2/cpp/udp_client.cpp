// Header file for input output functions
#include <iostream>
#include <stdlib.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sstream>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

extern void perror (const char *__s);
#define SUCCESS 0
#define FAILURE -1
#define MAXBUFFERSIZE 99999

int main(int argc, char const *argv[]) {
        int socketFD;
        int received;
        int serverlen;
        struct sockaddr_in serveraddr;
        struct hostent *server;
        int total_files;
        char buffer[MAXBUFFERSIZE];
        char clientIP[INET6_ADDRSTRLEN];
        // -----------------------------------------------------------------------
        std::cout << "argc --> " << argc <<'\n';



        std::string server_host = argv[1];
        int server_port = atoi(argv[2]);
        total_files = argc - 3;
        std::string files[2];
        int i;
        for(i = 3; i < argc; i++) {
                files[i-3] = argv[i];
        }
        // std::cout << "server_host --> " << server_host<<'\n';
        // std::cout << "server_port --> " << server_port <<'\n';
        // std::cout << "total_files --> " << total_files <<'\n';
        // for(int i = 0; i < total_files; i++) {
        //         std::cout << "file " << i << " --> " << files[i]<< '\n';
        // }
        // std::stringstream ss;
        // ss << "GET /";
        // ss << files[0];
        // ss << " HTTP/1.1";
        // std::string temp;
        // temp = ss.str();
        // std::cout << "request --> " << temp.c_str() << " and the length is " << temp.length() <<'\n';
        //
        socketFD = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketFD < 0) {
                std::perror("Error while opening the socket");
                return EXIT_FAILURE;
        }

        server = gethostbyname(server_host.c_str());
        if (server == NULL) {
                std::cerr << "no host --> " << server_host.c_str() << '\n';
                return EXIT_FAILURE;
        }
        /* build the server's Internet address */
        bzero((char *) &serveraddr, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
              (char *)&serveraddr.sin_addr.s_addr, server->h_length);
        serveraddr.sin_port = htons(server_port);
        std::cout << "Server ==> " <<  server->h_addr << '\n';

        serverlen = sizeof(serveraddr);
        std::stringstream request;
        request << "GET /";
        request << files[0];
        request << " HTTP/1.1\n";
        std::string temp_str;
        temp_str = request.str();
        std::cout << "request --> " << request.str().c_str() << '\n';
        if (sendto(socketFD, request.str().c_str(), request.str().length(), 0, (struct sockaddr *)&serveraddr, serverlen) < 0) {
                perror("sending error");
                return -1;
        }
        int size = 0;
        while(1) {
                if ((received = recvfrom(socketFD, buffer, MAXBUFFERSIZE, 0, (struct sockaddr *)&serveraddr, (socklen_t *)&serverlen)) == FAILURE ) {
                        perror("receive error");
                        exit(-1);
                }
                if (received == 0) {
                        break;
                }
                size += received;
                std::cout << buffer << '\n';
                memset(buffer, 0, sizeof buffer);

        }
        std::cout << "total received" << size <<'\n';
        close(socketFD);
        return 0;
}
// -----------------------------------------------------------------------