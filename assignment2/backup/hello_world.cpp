// Simple C++ program to display "Hello World"

// Header file for input output functions
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <stdlib.h>
using namespace std;

// main function -
// where the execution of program begins

#define SUCCESS 0
#define FAILURE -1

void getIPAddr() {
        string ip;
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
        // return ip;
}
int main(int argc, char *argv[]) {
        getIPAddr();
        return 0;
        std::cout << "number of arguments --> " << argc << '\n';
        // for (int i = 0; i < argc; i++) {
        //         std::cout << "arguments --> " << argv[i] << '\n';
        // }
        struct addrinfo hints, *res, *p;
        int status;
        char ipstr[INET_ADDRSTRLEN];
        if (argc != 2) {
                // fprintf(stderr, "usage: showip hostname\n");
                std::cerr << "usage: showip hostname" << '\n';
                return 1;
        }
        memset(&hints, 0, sizeof hints);

        hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
        hints.ai_socktype = SOCK_STREAM; // TCP OR UDP
        hints.ai_flags = AI_PASSIVE;     // fill in my IP for me // Did not do because I have provided the IP address
        if ((status = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
                // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                std::cerr << "getaddrinfo:" << gai_strerror(status) << '\n';
                return 2;
        }

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        for (p = res; p != NULL; p = p->ai_next) {
                void *addr;
                string ipver;
                if (p->ai_family == AF_INET) {
                        int sockfd;
                        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                        addr = &(ipv4->sin_addr);

                        // std::cout << "*ipv4 --> " << (short)ipv4->sin_family <<'\n';
                        // socket(IPv4 or IPv6, , int);
                        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                        std::cout << "family --> " << res->ai_family << " | socktype --> " << res->ai_socktype
                                  << " | protocol --> "<< res->ai_protocol << "\naddress --> " << res->ai_addr->sa_data << " | sa_family --> "
                                  << res->ai_addr->sa_family << " | length --> "<< res->ai_addr->sa_len << '\n';
                        std::cout << "socket descriptor --> " << sockfd << '\n';
                        // bind it to the port we passed in to getaddrinfo():

                        // int bind_result;
                        // int connect_return;
                        bind(sockfd, res->ai_addr, res->ai_addrlen);
                        // connect_return = connect(sockfd, res->ai_addr, res->ai_addrlen);
                        // std::cout << "connect_return --> " << connect_return << '\n';
                        ipver = "IPv4";
                } else {
                        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                        addr = &(ipv6->sin6_addr);
                        // std::cout << "*ipv6 --> " << (short)ipv6->sin6_family <<'\n';
                        ipver = "IPv6";
                }
                // Convert the IP to a string and print it.

                inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
                if (p->ai_family == AF_INET) {
                        std::cout << "IP Addresses for " << argv[1] << ": " << flush;
                        std::cout << ipver << " --> " << ipstr << '\n';
                }
        }
        freeaddrinfo(res);
        std::cout << '\n';
        return 0;
}
