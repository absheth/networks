// Header file for input output functions
#include <stdio.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <errno.h>
#include <ctype.h>


void getIPAddr();
void *get_in_addr(struct sockaddr *sa);
// #define PORT "9158"  // the client will be connecting to
#define SUCCESS 0
#define FAILURE -1
#define MAXBUFFERSIZE 1024 // 100
#define PERSISTENT 1
#define NONPERSISTENT 2

int main(int argc, char const *argv[]) {

        printf("argc --> %d\n", argc);
        // -----------------------------------------------------------------------
        int socketFD;
        int numbytes = 1024; // Initialization
        int connection_type;
        char buffer[MAXBUFFERSIZE];
        char request[MAXBUFFERSIZE];
        struct addrinfo hints, *server_info, *pointer;
        int getAddrRet;
        char clientIP[INET6_ADDRSTRLEN];
        if (argc != 5) {
                fprintf(stderr, "%s\n", "usage: client server_host server_port connection_type filename.txt");
                exit(1);
        }
        connection_type = atoi(argv[3]);
        const char *filename = argv[4];
        printf("%s\n", filename);

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if ((getAddrRet = getaddrinfo(argv[1], argv[2], &hints,  &server_info))!= SUCCESS) {
                fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(getAddrRet));
                return 1;
        }

        // loop through all the results and connect to the first we can
        for (pointer = server_info; pointer != NULL; pointer = pointer->ai_next) {

                if ((socketFD = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol)) == FAILURE ) {
                        perror("client socket error");
                        continue;
                }
                printf("Socket --> 1 %d\n", socketFD);
                printf("pointer->ai_addrlen --> %d \n",pointer->ai_addrlen );
                int x;
                if ((x = connect(socketFD,pointer->ai_addr, pointer->ai_addrlen)) == FAILURE) {
                        printf("%d\n", x);
                        close(socketFD);
                        perror("client connect");
                        continue;
                }
                                        printf("%d\n", x);
                printf("Socket --> 2 %d\n", socketFD);
                printf("Akash%d\n", pointer == NULL);
                break;
        }
        //printf("AKASH\n");
        if (pointer == NULL) {

                fprintf(stderr, "%s\n", "client: failed to connect");
                return 2;
        }

        inet_ntop(pointer->ai_family, get_in_addr((struct sockaddr *)&pointer->ai_addr), clientIP, sizeof clientIP);

        printf("Client: connecting to %s\n", clientIP);

        freeaddrinfo(server_info); // All done with this structure
        //char httphead[] = "GET /1mb.txt HTTP/1.1\n";
        char httphead[] = "GET /";
        strcpy(request, httphead);
        strcpy(&request[strlen(request)], filename);
        strcpy(&request[strlen(request)], " HTTP/1.1\n");
        strcpy(&request[strlen(request)], connection_type == PERSISTENT?"Connection: Keep-Alive\n":"Connection: Close\n");
        printf("REQUEST TO BE SENT --> %s\n", request);
        //exit(1);
        if (send(socketFD, request, strlen(request), 0) == FAILURE) {
                perror("send error");
        }
        /*if (connection_type == PERSISTENT) {
                if (send(socketFD, "Connection: keep-alive\n", strlen("Connection: keep-alive\n"), 0) == FAILURE) {
                        perror("send error");
                }
        } else {
                if (send(socketFD, "Connection: close\n", strlen("Connection: close\n"), 0) == FAILURE) {
                        perror("send error");
                }
        }*/
        // send(socketFD, "GET /message.html HTTP/1.1\n", 27, 0);
        while (1) {

                memset(buffer, 0, sizeof(buffer));
                numbytes = recv(socketFD, buffer, MAXBUFFERSIZE, 0);
                if ( numbytes == FAILURE ) {
                        perror("receive error");
                        break;
                }

                printf("Received from server --> \n %s \n", buffer);
                if (numbytes == 0) {
                        break;
                }

        }

        // shutdown(socketFD, SHUT_RDWR);
        printf("xxx\n");
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
