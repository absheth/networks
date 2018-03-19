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

#define SUCCESS 0
#define FAILURE -1
#define MAXBUFFERSIZE 1000000000  // 999999999
#define PACKET_SIZE 1472
// -----------------------------------------------------------------------
struct pkt {
    unsigned int seq_num;
    unsigned int ackno;
    char ack_flg;
    char *load;
};

extern void perror(const char *__s);
unsigned int char_to_int(char *p_charstream, int offset);
unsigned short char_to_short(char *p_charstream, int offset);
void number_to_char(char *p_char, unsigned short p_short_num, unsigned int p_seqnum,
                    unsigned int p_ackno, int p_offset);
void init_packet(pkt *p_packet);
// ------------------------------
unsigned int current_seqnum;
unsigned short receive_window;
unsigned int current_ackno;
char padding = '-';
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
    pkt rqst_packet;
    init_packet(&rqst_packet);
    current_seqnum = rqst_packet.seq_num;

    rqst_packet.load[0] = padding;
    rqst_packet.load[1] = rqst_packet.ack_flg;
    rqst_packet.seq_num = 0;
    rqst_packet.ackno = 0;
    receive_window = 65535;
    number_to_char(rqst_packet.load, receive_window, rqst_packet.seq_num, rqst_packet.ackno, 2);
    std::string http_rqst = "1mb.txt";
    // std::string http_rqst = "1234.txt";
    memcpy(rqst_packet.load + 12, http_rqst.c_str(), http_rqst.length());

    unsigned short l_rwin = char_to_short(rqst_packet.load, 2);
    std::cout << "l_rwin--> " << l_rwin << std::endl;

    unsigned int l_seq = char_to_int(rqst_packet.load, 4);
    std::cout << "l_seq--> " << l_seq << std::endl;

    unsigned int l_ack = char_to_int(rqst_packet.load, 8);
    std::cout << "l_ack--> " << l_ack << std::endl;

    //***************************************************
    auto start = std::chrono::high_resolution_clock::now();
    if (sendto(socketFD, rqst_packet.load, PACKET_SIZE, 0, (struct sockaddr *)&serveraddr,
               serverlen) < 0) {
        std::free(rqst_packet.load);
        perror("sending error");
        return -1;
    }
    std::free(rqst_packet.load);
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
unsigned int char_to_int(char *p_charstream, int p_offset) {
    // char temp[4];
    // memset(temp, 0, 4);
    // temp[0] = p_charstream[0];
    // temp[1] = p_charstream[1];
    // temp[2] = p_charstream[2];
    // temp[3] = p_charstream[3];
    // return (temp[0] << 24) | (temp[1] << 16) |
    //       (temp[2] << 8) | temp[3];
    return ((unsigned char)p_charstream[p_offset + 0] << 24) |
           ((unsigned char)p_charstream[p_offset + 1] << 16) |
           ((unsigned char)p_charstream[p_offset + 2] << 8) |
           (unsigned char)p_charstream[p_offset + 3];
}
/* Converts char array to unsigned hort */
unsigned short char_to_short(char *p_charstream, int p_offset) {
    return ((unsigned char)p_charstream[p_offset + 0] << 24) |
           (unsigned char)p_charstream[p_offset + 1];
}

/* Convert unsigned into char array of 4 bytes */
void number_to_char(char *p_char, unsigned short p_short_num, unsigned int p_seqnum,
                    unsigned int p_ackno, int p_offset) {
    // converting receive_window
    std::cout << "p_short_num --> " << p_short_num << std::endl;

    p_char[p_offset + 0] = (p_short_num >> 8) & 0xFF;
    p_char[p_offset + 1] = p_short_num & 0xFF;

    // converting current_seqnum
    p_char[p_offset + 2] = (p_seqnum >> 24) & 0xFF;
    p_char[p_offset + 3] = (p_seqnum >> 16) & 0xFF;
    p_char[p_offset + 4] = (p_seqnum >> 8) & 0xFF;
    p_char[p_offset + 5] = p_seqnum & 0xFF;

    // converting ackno
    p_char[p_offset + 6] = (p_ackno >> 24) & 0xFF;
    p_char[p_offset + 7] = (p_ackno >> 16) & 0xFF;
    p_char[p_offset + 8] = (p_ackno >> 8) & 0xFF;
    p_char[p_offset + 9] = p_ackno & 0xFF;
}
// -----------------------------------------------------------------------
void init_packet(pkt *p_packet) {
    p_packet->seq_num = 0;
    p_packet->ackno = 0;
    p_packet->ack_flg = '0';
    // int load_size = (PACKET_SIZE - ((2*sizeof(char)) + (2*sizeof(unsigned int) + sizeof(unsigned
    // short))));
    int load_size = PACKET_SIZE;
    p_packet->load = (char *)calloc(load_size, sizeof(char));
    memset(p_packet->load, 0, PACKET_SIZE);
}
