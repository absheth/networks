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
#define MAXBUFFERSIZE 99999 // 100
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
                files[i-4] = "GET /";
                files[i-4] = files[i-4] + argv[i];
                files[i-4] = files[i-4] + " HTTP/1.1\n";
                files[i-4] = files[i-4] + (connection_type == PERSISTENT?"Connection: Keep-Alive\n":"Connection: Close\n");
        }

        // INPUT DEBUG BLOCK;

        std::cout << "server_host --> " << server_host<<'\n';
        std::cout << "server_port --> " << server_port <<'\n';
        std::cout << "connection_type --> " << connection_type << (connection_type == PERSISTENT?"Connection: Keep-Alive\n":"Connection: Close\n" )<< '\n';
        std::cout << "total_files --> " << total_files <<'\n';
        for(int i = 0; i < total_files; i++) {
                std::cout << "file " << i << " --> " << files[i]<< '\n';
        }

        int socketFD;
        int numbytes = 1024; // Initialization

        char buffer[MAXBUFFERSIZE];
        // char request[MAXBUFFERSIZE];
        struct addrinfo hints, *server_info, *pointer;
        int getAddrRet;
        char clientIP[INET6_ADDRSTRLEN];



        connection_type = atoi(argv[3]);
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
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
                if (connection_type == PERSISTENT) {
                        if (connect(socketFD,pointer->ai_addr, pointer->ai_addrlen) == FAILURE) {
                                close(socketFD);
                                perror("client connect");
                                continue;
                        }
                }

                break;
        }

        if (pointer == NULL) {
                // fprintf(stderr, "%s\n", "client: failed to connect");
                std::cerr << "client: failed to connect " << '\n';
                exit(EXIT_FAILURE);
        }

        // inet_ntop(pointer->ai_family, get_in_addr((struct sockaddr *)&pointer->ai_addr), clientIP, sizeof clientIP);


        // std::cout <<"Client: connecting to "<< clientIP << '\n';
        // freeaddrinfo(server_info); // All done with this structure
        //char httphead[] = "GET /1mb.txt HTTP/1.1\n";
        for (i = 0; i < total_files; i++) {
                std::cout << "Request " << (i + 1) << " --> "<< '\n';
                std::cout << files[i] << '\n';
                if (connection_type == NONPERSISTENT) {
                        if ((socketFD = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol)) == FAILURE ) {
                                perror("client socket error");
                                return EXIT_FAILURE;
                        }
                        if (connect(socketFD,pointer->ai_addr, pointer->ai_addrlen) == FAILURE) {
                                close(socketFD);
                                perror("client connect");
                                return EXIT_FAILURE;
                        }
                }

                if (send(socketFD, files[i].c_str(), files[i].length(), 0) == FAILURE) {
                        perror("send error");
                        close(socketFD);
                        continue;
                }
                while (1) {
                        if ((numbytes = recv(socketFD, buffer, MAXBUFFERSIZE, 0)) == FAILURE ) {
                                perror("receive error");
                                exit(-1);
                        }
                        // std::cout << "numbytes --> " << numbytes << '\n';
                        // std::cout << "HERE --> " << buffer[strlen(buffer)-1] << '\n';
                        // if (strcmp(&buffer[strlen(buffer)-1], "x") == 0) {
                        //         std::string stemp = buffer;
                        //         std::cout << stemp.substr(0,strlen(buffer)-1) << '\n';
                        //         sleep(2);
                        //         close(socketFD);
                        //         break;
                        // }
                        // std::cout << buffer << strlen(buffer)<< '\n';

                        if (numbytes == 0) {
                                std::cout << "Numbyte revceived --> " << numbytes << '\n';
                                if (connection_type == NONPERSISTENT) {
                                        close(socketFD);
                                        // std::perror("CLOSE FAILED");
                                }
                                std::cout << "breaking" << '\n';
                                break;
                        }
                        if (connection_type == PERSISTENT) {
                                if (strcmp(&buffer[strlen(buffer)-2], "x!") == 0) {

                                        std::cout << "x! FROM SERVER" << '\n';
                                        std::string stemp = buffer;
                                        std::cout << stemp.substr(0,strlen(buffer)-2) << '\n';
                                        // sleep(2);
                                        // close(socketFD);
                                        break;
                                }
                        }
                        std::cout << buffer << '\n';

                        // std::cout << "HERE --> " << buffer[strlen(buffer)-1] << '\n';
                        memset(buffer, 0, sizeof buffer);
                        // std::cout << "STILL INSIDE THE WHILE LOOP" << '\n';
                }
                // std::cout << "OUT OF THE WHILE LOOP" << '\n';
                memset(buffer, 0, sizeof buffer);
        }
        // std::cout << "OUT OF FOR LOOP" << '\n';
        if (connection_type == PERSISTENT) {
                send(socketFD,"!!!!!", 5, 0);
                // if (send(socketFD,"end", 3, 0) == FAILURE) {
                //         perror("send error");

                // }
                close(socketFD);
        }



        // shutdown(socketFD, SHUT_RDWR);

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