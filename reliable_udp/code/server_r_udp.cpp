// Header file for input output functions
#include <iostream>
// #include <stdio.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
// -----------------------------------------------------------------------

#define PACKET_SIZE 1472
// struct message {
//     char data[DATA_SIZE];
// };

struct pkt {
    unsigned int seq_num;
    char ack_flg;  // THINK
    char *load;
};
unsigned int chartoint(char *p_charstream, int offset);
void inttochar(char *p_char, unsigned int p_number, int offset);
void parse_packet(char *p_receive_buffer, pkt *p_packet);
// -----------------------------------------------------------------------
void perror(const char *str);
void service_request(int connFD);
void quit(std::string p_error_message);
int Trim(char *buffer);
int start_server();
ssize_t Writeline(int sockd, const void *vptr, size_t n);
int StrUpper(char *buffer);
void trim(std::string &s);
#define PORT 9158
#define SUCCESS 0
#define FAILURE -1
#define BACKLOG 5
#define MAXBUFFERSIZE 255
#define PERSISTENT 1
#define NONPERSISTENT 2

std::string working_directory;
int listen_socket;
struct sockaddr_in clientAddr;
char clientIP[INET6_ADDRSTRLEN];
struct sockaddr_in clientaddr;
int clientlen;
struct hostent *hostp;
char *hostaddrp;
int main(int argc, char const *argv[]) {
    std::cout << "argc --> " << argc << '\n';
    working_directory = getenv("PWD");
    std::cout << "working_directory --> " << working_directory << '\n';
    //--------------------------------------------
    if (start_server() != 0) {
        std::cout << "Server did not start." << '\n';
        return EXIT_FAILURE;
    }
    //--------------------------------------------
    int count = 0;
    clientlen = sizeof(clientaddr);
    while (1) {
        service_request(listen_socket);
        std::cout << "******HERE******" << '\n';
        std::cout << "count--> " << count++ << '\n';
        std::cout << '\n';
    }
    return 0;
}

// -----------------------------------------------------------------------
void service_request(int connFD) {
    int sum = 0;
    std::cout << '\n';
    std::cout << "SERVING NOW.." << '\n';
    char receive_buffer[PACKET_SIZE];
    int received_bytes = 0;
    memset(receive_buffer, 0, PACKET_SIZE);
    received_bytes = recvfrom(connFD, receive_buffer, PACKET_SIZE, 0,
                              (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen);
    std::cout << "received_bytes--> " << received_bytes << std::endl;
    pkt received_pkt;
    parse_packet(receive_buffer, &received_pkt);
    unsigned int seq_num = chartoint(receive_buffer, 1);
    std::cout << "seq_num --> " << seq_num << std::endl;
    std::cout << "Request --> " << received_pkt.load[0] << std::endl;
    
    
    std::free(received_pkt.load); // FREE MEMORY
    return;
    if (received_bytes < 0) {
        
    } else if (received_bytes > MAXBUFFERSIZE) {
        
    }
    
    // return 0;
}

// -----------------------------------------------------------------------
int Trim(char *buffer) {
    int n = strlen(buffer) - 1;
    while (!isalnum(buffer[n]) && n >= 0) buffer[n--] = '\0';
    return 0;
}
// -----------------------------------------------------------------------
void quit(std::string p_error_message) {
    std::cerr << "p_error_message --> " << p_error_message << '\n';
    exit(EXIT_FAILURE);
}
// -----------------------------------------------------------------------
/*  Converts a string to upper-case  */
int StrUpper(char *buffer) {
    while (*buffer) {
        *buffer = toupper(*buffer);
        ++buffer;
    }
    return 0;
}

// -----------------------------------------------------------------------
void trim(std::string &s) {
    size_t p = s.find_first_not_of(" \t");
    s.erase(0, p);
    p = s.find_last_not_of(" \t");
    if (std::string::npos != p)
        s.erase(p + 1);
}
// -----------------------------------------------------------------------

int start_server() {
    struct sockaddr_in server; /* server's addr */
    int enable = 1;
    if ((listen_socket = socket(AF_INET, SOCK_DGRAM, 0)) == FAILURE) {
        std::perror("Socket Error ");
        return EXIT_FAILURE;
    }
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
        std::perror("setsockopt error");
        return FAILURE;
    }
    bzero((char *)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    if (bind(listen_socket, (struct sockaddr *)&server, sizeof(server)) == -1) {
        close(listen_socket);  // close the listening socket because the binding failed.
        std::perror("server bind error");
        return EXIT_FAILURE;
    }
    std::cout << "Server started. Waiting for the connection." << '\n';
    std::cout << '\n';
    return SUCCESS;
}

// -----------------------------------------------------------------------
/* Convert unsigned char array to int */
unsigned int chartoint(char *p_charstream, int offset) {
    unsigned char temp[4];
    std::memset(temp, 0, 4);
    temp[0] = p_charstream[offset + 0];
    temp[1] = p_charstream[offset + 1];
    temp[2] = p_charstream[offset + 2];
    temp[3] = p_charstream[offset + 3];
    unsigned int seq = (temp[0] << 24) | (temp[1] << 16) | (temp[2] << 8) | temp[3];

    return seq;

    // return ((unsigned char)p_charstream[offset+0] << 24) | ((unsigned char)p_charstream[offset+1]
    // << 16) | ((unsigned char)p_charstream[offset+2] << 8) | (unsigned char)p_charstream[offset+3];

    // return (p_charstream[0] << 24) | (p_charstream[1] << 16) | (p_charstream[2] << 8) |
    // p_charstream[3];
}
/* Convert unsigned into char array of 4 bytes */
void inttochar(char *p_char, unsigned int p_number, int p_offset) {
    // p_char[0] = char((p_number >> 24) & 0xFF);
    // p_char[1] = char((p_number >> 16) & 0xFF);
    // p_char[2] = char((p_number >> 8) & 0xFF);
    // p_char[3] = char(p_number & 0xFF);

    p_char[p_offset + 0] = (p_number >> 24) & 0xFF;
    p_char[p_offset + 1] = (p_number >> 16) & 0xFF;
    p_char[p_offset + 2] = (p_number >> 8) & 0xFF;
    p_char[p_offset + 3] = p_number & 0xFF;
    // printf("%x", p_char[0]);
}
// -----------------------------------------------------------------------
void parse_packet(char *p_receive_buffer, pkt *p_packet) {
    int load_size = (PACKET_SIZE - (sizeof(char) + sizeof(unsigned int)));
    p_packet->ack_flg = p_receive_buffer[0];
    p_packet->seq_num = chartoint(p_receive_buffer, 1);
    p_packet->load = (char *)calloc(load_size, sizeof(char));
    std::cout << "size of load --> " << load_size << std::endl;
    std::memset(p_packet->load, 0, load_size);
    std::memcpy(p_packet->load, p_receive_buffer + 5, load_size);
    std::cout << "p_packet->seq_num --> " << p_packet->seq_num << std::endl;
}
