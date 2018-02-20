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
void getIPAddr();
void *get_in_addr(struct sockaddr *sa);
// #define PORT "9158"  // the client will be connecting to
#define SUCCESS 0
#define FAILURE -1
#define MAXBUFFERSIZE 999999 // 100
#define PERSISTENT 1
#define NONPERSISTENT 2

int main(int argc, char const *argv[]) {

        std::cout << "argc --> " << argc <<'\n';
        int connection_type;
        int total_files;
        // -----------------------------------------------------------------------

        std::string server_host = argv[1];
        std::string server_port = argv[2];
        connection_type = atoi(argv[3]);
        total_files = argc - 4;
        std::string files[2];
        int i;
        for(i = 4; i < argc; i++) {
                files[i-4] = argv[i];
        }

        // INPUT DEBUG BLOCK;

        std::cout << "server_host --> " << server_host<<'\n';
        std::cout << "server_port --> " << server_port <<'\n';
        std::cout << "connection_type --> " << connection_type << (connection_type == PERSISTENT?"Connection: Keep-Alive\n":"Connection: Close\n" )<< '\n';
        std::cout << "total_files --> " << total_files <<'\n';
        for(int i = 0; i < total_files; i++) {
                std::cout << "file " << i << " --> " << files[i]<< '\n';
        }


        std::stringstream ss;
        ss << "GET /";
        ss << files[0];
        ss << " HTTP/1.1 \n";
        ss << (connection_type == PERSISTENT?"Connection: Keep-Alive\n":"Connection: Close\n");
        std::string temp;
        temp = ss.str();
        std::cout << "request --> " << temp.c_str() << " and the length is " << temp.length() <<'\n';
        // exit(1);



        int socketFD;
        int numbytes = 1024; // Initialization

        char buffer[MAXBUFFERSIZE];
        // char request[MAXBUFFERSIZE];
        struct addrinfo hints, *server_info, *pointer;
        int getAddrRet;
        char clientIP[INET6_ADDRSTRLEN];



        connection_type = atoi(argv[3]);
        // const char *filename = argv[4];
        // std::cout << "filename --> " << filename <<'\n';

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        if ((getAddrRet = getaddrinfo(server_host.c_str(), server_port.c_str(), &hints,  &server_info))) {
                std::cerr << "getaddrinfo error:" << gai_strerror(getAddrRet) << '\n';
                return 1;
        }

        // loop through all the results and connect to the first we can
        for (pointer = server_info; pointer != NULL; pointer = pointer->ai_next) {
                if ((socketFD = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol)) == FAILURE ) {
                        perror("client socket error");
                        continue;
                }
                // if (connect(socketFD,pointer->ai_addr, pointer->ai_addrlen) == FAILURE) {
                //         close(socketFD);
                //         perror("client connect");
                //         continue;
                // }
                break;
        }

        if (pointer == NULL) {
                // fprintf(stderr, "%s\n", "client: failed to connect");
                std::cerr << "client: failed to connect " << '\n';
                return 2;
        }

        inet_ntop(pointer->ai_family, get_in_addr((struct sockaddr *)&pointer->ai_addr), clientIP, sizeof clientIP);


        std::cout <<"Client: connecting to "<< clientIP << '\n';
        freeaddrinfo(server_info); // All done with this structure
        //char httphead[] = "GET /1mb.txt HTTP/1.1\n";

        // char httphead[] = "GET /";
        // strcpy(request, httphead);
        // strcpy(&request[strlen(request)], filename);
        // strcpy(&request[strlen(request)], " HTTP/1.1\n");
        // strcpy(&request[strlen(request)], connection_type == PERSISTENT?"Connection: Keep-Alive\n":"Connection: Close\n");
        // printf("REQUEST TO BE SENT --> %s\n", request);

        std::stringstream request;
        request << "GET /";
        request << files[0];
        request << " HTTP/1.1 \n";
        request << (connection_type == PERSISTENT?"Connection: Keep-Alive\n":"Connection: Close\n");
        std::string temp_str;
        temp_str = request.str();
        std::cout << "request --> " << request.str().c_str() << '\n';
        if (sendto(socketFD, request.str().c_str(), request.str().length(), 0, pointer->ai_addr, pointer->ai_addrlen) < 0) {
                perror("sending error");
                return -1;
        }

        // if (send(socketFD, request.str().c_str(), request.str().length(), 0) == FAILURE) {
        //         perror("send error");
        //         return -1;
        // }
        /*if (connection_type == PERSISTENT) {
                if (send(socketFD, "Connection: keep-alive\n", strlen("Connection: keep-alive\n"), 0) == FAILURE) {
                        std::perror("send error");
                }
        } else {
                if (send(socketFD, "Connection: close\n", strlen("Connection: close\n"), 0) == FAILURE) {
                        std::perror("send error");
                }
        }*/
        // send(socketFD, "GET /message.html HTTP/1.1\n", 27, 0);
        while ( numbytes !=0) {
                if ((numbytes = recvfrom(socketFD, buffer, MAXBUFFERSIZE, 0, pointer->ai_addr, &pointer->ai_addrlen)) == FAILURE ) {
                        perror("receive error");
                        exit(-1);
                }
                // printf("Received from server --> \n %s \n", buffer);
                std::cout << buffer << '\n';
                memset(buffer, 0, sizeof buffer);

        }

        // shutdown(socketFD, SHUT_RDWR);
        close(socketFD);
        return 0;
}
// -----------------------------------------------------------------------
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
        if (sa->sa_family == AF_INET) {
                return &(((struct sockaddr_in*)sa)->sin_addr);
        }
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
