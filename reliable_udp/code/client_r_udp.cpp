// Header file for input output functions
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <chrono>
#include <iostream>
#include <sstream>
extern void perror(const char *__s);
#define SUCCESS 0
#define FAILURE -1
#define MAXBUFFERSIZE 99999
// -----------------------------------------------------------------------

#define DATA_SIZE 1560
struct message {
    char data[DATA_SIZE];
};

struct pkt {
    int seq_num;
    int ack_num;
    char ack_flg;  // THINK
    char load[DATA_SIZE];
};

void make_packet_data(char *p_data_str, pkt p_packet);

// -----------------------------------------------------------------------
int main(int argc, char const *argv[]) {
    std::cout << '\n';
    std::cout << "---------------------------- START ----------------------------" << '\n';
    int socketFD;
    int received;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    int total_files;
    char buffer[MAXBUFFERSIZE];
    char clientIP[INET6_ADDRSTRLEN];
    // -----------------------------------------------------------------------
    std::cout << "argc --> " << argc << '\n';
    // std::string server_host = argv[1];
    // int server_port = atoi(argv[2]);
    std::string server_host = "localhost";
    int server_port = 9158;
    std::string http_rqst = "GET /1mb.txt HTTP/1.1";
    /*
    total_files = argc - 3;
    std::string files[2];
    int i;
    for (i = 3; i < argc; i++) {
        files[i - 3] = argv[i];
    }
    */
    // -----------------------------------------------------------------------
    socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    std::cout << "SOCKET --> " << socketFD << std::endl;
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
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(server_port);
    std::cout << "Server ==> " << server_host << '\n';
    std::cout << "Port ==> " << server_port << '\n';
    serverlen = sizeof(serveraddr);
    // -----------------------------------------------------------------------
    // TRIAL 
    pkt rqst_packet;
    rqst_packet.ack_num = -1;   // THINK
    rqst_packet.ack_flg = 'x';  // or a
    rqst_packet.seq_num = 0;    // THINK

    std::stringstream request;
    request << rqst_packet.ack_num;  // ERROR. NEEDS TO BE FIXED. CANNOT BE DONE THIS WAY.
    request << ",";
    request << rqst_packet.ack_flg;  // ERROR. NEEDS TO BE FIXED. CANNOT BE DONE THIS WAY.
    request << ",";
    request << rqst_packet.seq_num;  // ERROR. NEEDS TO BE FIXED. CANNOT BE DONE THIS WAY.
    request << ",";
    request << http_rqst;
    std::cout << "REQUEST --> " << request.str().c_str() << " | length = " << request.str().length()
              << std::endl;
    std::cout << "" << std::endl;
    strcpy(rqst_packet.load, request.str().c_str());
    std::cout << "rqst_packet.ack_num --> " << rqst_packet.ack_num << std::endl;
    std::cout << "rqst_packet.ack_flg --> " << rqst_packet.ack_flg << std::endl;
    std::cout << "rqst_packet.seq_num --> " << rqst_packet.seq_num << std::endl;
    std::cout << "rqst_packet.load --> " << rqst_packet.load << std::endl;

    return 0;

    auto start = std::chrono::high_resolution_clock::now();
    if (sendto(socketFD, request.str().c_str(), request.str().length(), 0,
               (struct sockaddr *)&serveraddr, serverlen) < 0) {
        perror("sending error");
        return -1;
    }
    int size = 0;
    std::cout << "------------ RESPONSE START ------------" << '\n';
    while (1) {
        if ((received = recvfrom(socketFD, buffer, MAXBUFFERSIZE, 0, (struct sockaddr *)&serveraddr,
                                 (socklen_t *)&serverlen)) == FAILURE) {
            perror("receive error");
            exit(-1);
        }
        std::string x(buffer);
        if (x == "**over**") {
            break;
        }
        size += received;
        std::cout << buffer << '\n';
        memset(buffer, 0, sizeof buffer);
    }

    close(socketFD);
    std::cout << "------------ RESPONSE END ------------" << '\n';
    std::cout << '\n';
    std::cout << "total received bytes--> " << size << '\n';
    std::cout << '\n';
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";
    // std::cout << '\n';
    std::cout << "---------------------------- END ----------------------------" << '\n';
    return 0;
}
// -----------------------------------------------------------------------
void make_packet_data(char *p_data_str, pkt p_packet) {
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;
}
