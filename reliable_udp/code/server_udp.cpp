// START OF SERVER FILE
// Header file for input output functions
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
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>

// -----------------------------------------------------------------------
// CONSTANTS
#define PORT 9158
#define PACKET_SIZE 1472
#define DATA_SIZE 1460
#define HEADER_SIZE 12
#define SUCCESS 0
#define FAILURE -1
// -----------------------------------------------------------------------
struct pkt {
    unsigned int seq_num;
    unsigned int ackno;
    char ack_flg;
    char *load;
};

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// METHOD_DECLARATIONS
unsigned int char_to_int(char *p_charstream, int offset);
unsigned short char_to_short(char *p_charstream, int offset);
void number_to_char(char *p_char, unsigned short p_short_num, unsigned int p_seqnum,
                    unsigned int p_ackno, int p_offset);
int write_data_to_file(char *p_received_data, int p_data_length, unsigned int p_packet_number);
void service_request(int p_listen_socket);
int start_server();
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
//  HEADER_FORMAT
//  Padding         --> -                   :: 0
//  ACK Flag        --> d: data             :: 1
//                  --> a: ack              :: 1
//  Receive Window  -->                     :: 2..3
//  Sequence        -->                     :: 4..7
//  Acknowledge     -->                     :: 8..11
//  Data starts     -->                     :: 11
// -----------------------------------------------------------------------

// VARIABLE_DECLARATIONS
int listen_socket;
struct sockaddr_in clientAddr;
char clientIP[INET6_ADDRSTRLEN];
struct sockaddr_in clientaddr;
int clientlen;
struct hostent *hostp;
char *hostaddrp;

// -----------------------------------------------------------------------
// MAIN_METHOD

int main(int argc, char const *argv[]) {
    std::cout << "AKASH SHETH - SERVER" << std::endl;
    std::cout << "argc --> " << argc << '\n';
    std::string working_directory = getenv("PWD");
    std::cout << "working_directory --> " << working_directory << '\n';
    // window_size = 9;
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
        std::cout << "------------------------------" << std::endl;
        std::cout << ".. REQUEST SERVED .." << std::endl;
        std::cout << "   COUNT --> " << count++ << '\n';
        std::cout << "------------------------------" << std::endl;
    }
    return 0;
}

void service_request(int p_listen_socket) {
LISTEN_AGAIN:
    // VARIABLE_DECLARATIONS
    int request_error = 0;
    char receive_buffer[PACKET_SIZE];
    int bytes_received = 0;
    unsigned int start_index = 0;
    memset(receive_buffer, 0, PACKET_SIZE);
    // -----------------------------
    std::cout << std::endl;
    std::cout << "SERVING NOW.. Waiting for the request. " << std::endl;
    bytes_received = recvfrom(p_listen_socket, receive_buffer, PACKET_SIZE, 0,
                              (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen);

    if (bytes_received <= 0) {
        std::cout << std::endl;
        std::cout << "BYTES READ --> " << bytes_received << std::endl;
        std::cout << "JUMP :: LISTEN_AGAIN " << std::endl;
        goto LISTEN_AGAIN;
    }

    start_index = char_to_int(receive_buffer, 4);

    if (start_index != 0) {
        std::cout << std::endl;
        std::cout << "REQUEST ERROR :: start_index --> " << std::endl;
        request_error = 1;
        std::cout << "JUMP :: END" << std::endl;
    }

    // -----------------------------
    // VARIABLE_DECLARATIONS
    //
END:
    total_packets_sent = 0;
    base = 0;
    
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
