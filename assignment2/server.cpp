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
#include "Request.h"
#include <string.h>


void getIPAddr();
void sigchild_handler(int s);
void serviceRequest(int connFD);
void *get_in_addr(struct sockaddr *sa);
#define PORT "9158"  // the port users will be connecting to
#define SUCCESS 0
#define FAILURE -1
#define BACKLOG 5
#define MAXBUFFERSIZE 1024

int main(int argc, char const *argv[]) {

        printf("argc --> %d\n", argc);
        std::cout << "argc --> "<< argc << '\n';
        getIPAddr();
        // -----------------------------------------------------------------------
        int enable = 1;
        int listen_socket, communicate_socket;  // listen on sock_fd, new connection on new_fd
        struct addrinfo hints, *server_info, *pointer;
        struct sockaddr_storage clientAddr; // This stores the client address information.
        socklen_t sin_size;
        struct sigaction sa; // NO IDEA WHY USED
        int getAddrRet;
        char clientIP[INET6_ADDRSTRLEN];
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM; //SOCK_STREAM, SOCK_DGRAM
        hints.ai_flags = AI_PASSIVE; // USE MY IP
        if ((getAddrRet = getaddrinfo(NULL, PORT, &hints, &server_info)) != SUCCESS ) {
                // fprintf(stderr, "getaddrinfo error --> %s\n", gai_strerror(getAddrRet));
                std::cerr << "getaddrinfo error -->" << gai_strerror(getAddrRet) << '\n';
                return 1; // CHECK WHY 1;
        }
        // loop through all the results and bind to the first we can
        for (pointer = server_info; pointer != NULL; pointer = server_info->ai_next) {
                // printf("pointer family --> %s\n", (pointer->ai_family == AF_INET ? "AF_INET" : "AF_INET6" ));
                std::cout << "pointer family --> " << (pointer->ai_family == AF_INET ? "AF_INET" : "AF_INET6" ) << '\n';
                // printf("socket type --> %d\n", pointer->ai_socktype);
                std::cout << "socket type --> " << pointer->ai_socktype << '\n';
                // printf("protocol --> %d\n", pointer->ai_protocol); // 6 because TCP, 17 because UDP
                std::cout << "protocol --> " << pointer->ai_protocol << '\n'; // 6 because TCP, 17 because UDP

                if ((listen_socket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol)) == FAILURE) {
                        perror("## Socket Error ##");
                        continue; // because we want to keep checking for other IP Addresses.
                }
                if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
                        perror("setsockopt error");
                        exit(1);
                }


                if (bind(listen_socket, pointer->ai_addr, pointer->ai_addrlen) == -1) {
                        close(listen_socket);
                        perror("server bind error");
                        continue;
                }
                break;
        }

        if (pointer == NULL) {
                fprintf(stderr, "Server failed to bind\n");
                return 2;
        }
        freeaddrinfo(server_info); // USE COMPLETE

        if (listen(listen_socket, BACKLOG) == FAILURE) {
                perror("listen error");
                exit(1);
        }
        // NO IDEA WHY USED - START
        sa.sa_handler = sigchild_handler; // reap all dead processes
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                perror("sigaction");
                exit(1);
        }
        // NO IDEA WHY USED - END
        // printf("Server started. Waiting for the connection.\n");
        std::cout << "Server started. Waiting for the connection" << '\n';
        while (1) {
                sin_size = sizeof clientAddr;
                communicate_socket = accept(listen_socket, (struct sockaddr *)&clientAddr, &sin_size);
                if (communicate_socket == FAILURE) {
                        perror("Accept error");
                        continue;
                }

                printf("Communication file descriptor --> %d\n", communicate_socket);
                std::cout << "Communication file descriptor --> " << communicate_socket << '\n';
                inet_ntop(clientAddr.ss_family, get_in_addr((struct sockaddr *)&clientAddr), clientIP, sizeof clientIP);

                printf("Connection from --> %s\n", clientIP);
                std::cout << "Connection from --> " << clientIP << '\n';

                if (!fork()) { // This is a child process;
                        close(listen_socket); // Child doesn't need the listener
                        serviceRequest(communicate_socket);
                        // if (send(communicate_socket, "Akash Sheth is a good boy.", sizeof "Akash Sheth is a good boy.", 0) == FAILURE) {
                        //         perror("send error");
                        // }
                        close(communicate_socket);
                        exit(0);
                }
                close(communicate_socket); // Parent does not need this.
        }

        // -----------------------------------------------------------------------

        return 0;
}

// -----------------------------------------------------------------------
void sigchild_handler(int s)
{
        while(waitpid(-1, NULL, WNOHANG) > 0) ;
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
// -----------------------------------------------------------------------
void serviceRequest(int connFD) {
        HttpRequest current_request;
        // current_request.initialize_request();
        current_request.initialize_request();

        // Read the http request.
        if (current_request.read_request(connFD) == FAILURE) {
                std::cout << "FAILURE" << '\n';
        } else {
                std::cout << "SUCCESS" << '\n';
        }
}
// -----------------------------------------------------------------------// -----------------------------------------------------------------------

void getIPAddr() {
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1) {
                perror("getifaddrs");
                exit(EXIT_FAILURE);
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
                family = ifa->ifa_addr->sa_family;

                if (family == AF_INET) {
                        s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                        host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                        if (s != 0) {
                                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                                exit(EXIT_FAILURE);
                        }
                        printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);
                }
        }
}
