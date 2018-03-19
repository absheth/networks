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
#include <climits>
#include <iostream>
#include <sstream>

extern void perror(const char *__s);
unsigned int chartoint(char *p_charstream, int offset);
void inttochar(char *p_char,unsigned int p_number, int offset);

#define SUCCESS 0
#define FAILURE -1
#define MAXBUFFERSIZE 1000000000// 999999999 

#define PACKET_SIZE 1472
// -----------------------------------------------------------------------
struct pkt {
    unsigned int seq_num;
    char ack_flg;   
    char load[PACKET_SIZE];
};


// -----------------------------------------------------------------------
int main(int argc, char const *argv[]) {
    std::cout << "AKASH SHETH" << std::endl;
    std::cout << '\n';
    std::cout << "---------------------------- START ----------------------------" << '\n';
    int socketFD;
    int received;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    // -----------------------------------------------------------------------
    std::string server_host = "localhost";
    int server_port = 9158;
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
    
  
    // MAKE REQUEST
    std::string http_rqst = "rqst:1mb.txt";
    pkt rqst_packet;
    rqst_packet.ack_flg = '0'; 
    rqst_packet.seq_num = 54;
    std::memset(rqst_packet.load, 0, PACKET_SIZE);
    rqst_packet.load[0] = rqst_packet.ack_flg;
    inttochar(rqst_packet.load, rqst_packet.seq_num, 1);
    std::memcpy(rqst_packet.load+5, http_rqst.c_str(), http_rqst.length());
    unsigned int a = chartoint(rqst_packet.load, 1);
    std::cout << "Converted back --> " << a << std::endl;
    
    //***************************************************
    auto start = std::chrono::high_resolution_clock::now();
    if (sendto(socketFD, rqst_packet.load, PACKET_SIZE, 0,
               (struct sockaddr *)&serveraddr, serverlen) < 0) {
        perror("sending error");
        return -1;
    }

    return 0;
    int size = 0;
    std::cout << "------------ RESPONSE START ------------" << '\n';
   /* while (1) {
        if ((received = recvfrom(socketFD, buffer, PACKET_SIZE, 0, (struct sockaddr *)&serveraddr,
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
    }*/

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
    //***************************************************
    return 0;
}
// -----------------------------------------------------------------------
/* Convert unsigned char array to int */
unsigned int chartoint(char* p_charstream, int offset) {
    // char temp[4];
    // std::memset(temp, 0, 4);
    // temp[0] = p_charstream[0];
    // temp[1] = p_charstream[1];
    // temp[2] = p_charstream[2];
    // temp[3] = p_charstream[3];
    // return (temp[0] << 24) | (temp[1] << 16) |
    //       (temp[2] << 8) | temp[3];


    return ((unsigned char)p_charstream[offset+0] << 24) | ((unsigned char)p_charstream[offset+1] << 16) |
          ((unsigned char)p_charstream[offset+2] << 8) | (unsigned char)p_charstream[offset+3];

    // return (p_charstream[0] << 24) | (p_charstream[1] << 16) | (p_charstream[2] << 8) | p_charstream[3];
}


/* Convert unsigned into char array of 4 bytes */
void inttochar(char* p_char, unsigned int p_number, int offset) {
    std::cout << "p_number --> " << p_number << std::endl;
         
    p_char[offset+0] = (p_number >> 24) & 0xFF;
    p_char[offset+1] = (p_number >> 16) & 0xFF;
    p_char[offset+2] = (p_number >> 8) & 0xFF;
    p_char[offset+3] = p_number & 0xFF;
    // printf("%x", p_char[0]);
}
// -----------------------------------------------------------------------
