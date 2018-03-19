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
#define DATA_SIZE 1460
// struct message {
//     char data[DATA_SIZE];
// };

struct pkt {
    unsigned int seq_num;
    unsigned int ackno;
    char ack_flg;
    char *load;
};
unsigned int char_to_int(char *p_charstream, int offset);
unsigned short char_to_short(char *p_charstream, int offset);
void number_to_char(char *p_char, unsigned short p_short_num, unsigned int p_seqnum,
                    unsigned int p_ackno, int p_offset);
void init_packet(pkt *p_packet);
void parse_packet(char *p_receive_buffer, pkt *p_packet);
// -----------------------------------------------------------------------
unsigned int current_seqnum = 0;
unsigned short receive_window;
unsigned int current_ackno = 0;
char padding = '-';
int window_size;

// -----------------------------------------------------------------------
void perror(const char *str);
void service_request(int connFD);
void quit(std::string p_error_message);
int Trim(char *buffer);
int start_server();
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
    // window_size = atoi(argv[1]);
    window_size = 2;
    //--------------------------------------------
    if (start_server() != 0) {
        std::cout << "Server did not start." << '\n';
        return EXIT_FAILURE;
    }
    //--------------------------------------------
    int count = 0;
    clientlen = sizeof(clientaddr);
    // while (1) {
    service_request(listen_socket);
    std::cout << "******HERE******" << '\n';
    std::cout << "count--> " << count++ << '\n';
    std::cout << '\n';
    // }
    return 0;
}

// -----------------------------------------------------------------------
void service_request(int connFD) {
    /* int sum = 0;
    std::cout << '\n';
    std::cout << "SERVING NOW.." << '\n';
    char receive_buffer[PACKET_SIZE];
    int received_bytes = 0;
    memset(receive_buffer, 0, PACKET_SIZE);
    received_bytes = recvfrom(connFD, receive_buffer, PACKET_SIZE, 0,
                              (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen);
    std::cout << "received_bytes--> " << received_bytes << std::endl;
    pkt received_pkt;
    init_packet(&received_pkt);
    parse_packet(receive_buffer, &received_pkt);
    // unsigned int seq_num = char_to_int(receive_buffer, 4);
    std::cout << "seq_num --> " << received_pkt.seq_num << std::endl;
    std::cout << "Request --> " << received_pkt.load << std::endl;
    std::cout << "Request length --> " << strlen(received_pkt.load) << std::endl;
    std::cout << "ackno --> " << received_pkt.ackno << std::endl;
    std::cout << "receive_window--> " << receive_window << std::endl;
    std::cout << "ackflg --> " << received_pkt.ack_flg << std::endl;
    // --------------------
    FILE *f = fopen(received_pkt.load, "rb");*/
    FILE *f = fopen("1mb.txt", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *file = (char *)malloc(fsize + 1);
    fread(file, fsize, 1, f);
    fclose(f);
    // std::free(received_pkt.load);  // FREE MEMORY
    // -------------------_
    file[fsize] = 0;
    // std::cout << "FILE--> " << file << std::endl;
    std::cout << "FILE SIZE --> " << fsize << std::endl;

    // unsigned int l_seq_no = received_pkt.seq_num;
    // unsigned int l_ack_no = received_pkt.ackno;

    unsigned int l_seq_no = 0;
    unsigned int l_ack_no = 0;
    char l_ack_flg = '0';
    unsigned int l_waiting_seq_no = 0;
    int window_count = 0;
    int isWaitingSet = 0;
    while (current_seqnum < fsize) {
        // First, send packets equal to the window size
    NEXT_PACKET:
        while (window_count < window_size) {
            /*pkt send_pkt;
            init_packet(&send_pkt);
            send_pkt.seq_num = current_seqnum;
            send_pkt.ackno = current_ackno;
            send_pkt.ack_flg = '0';
            send_pkt.load[0] = padding;
            send_pkt.load[1] = send_pkt.ack_flg;
            number_to_char(send_pkt.load, receive_window, send_pkt.seq_num, send_pkt.ackno, 2);
            memcpy(send_pkt.load + 12, file + current_seqnum, DATA_SIZE);*/
            // ********************************************

            // ********************************************

            if (!isWaitingSet) {
                l_waiting_seq_no = DATA_SIZE;
                isWaitingSet = 1;
            }
            current_seqnum += DATA_SIZE;
            window_count++;

            std::cout << "current_seqnum -->" << current_seqnum << std::endl;
            std::cout << "l_waiting_seq_no --> " << l_waiting_seq_no << std::endl;
            std::cout << std::endl;
        }
        // TIMER GOES SOMEWHERE HERE

        // GOTO RECEIVE BLOCKING CALL : recvfrom();

        // CHeck for the ack flag .
        // Suppose you receive a packet with an ack = 1 and service_request
        // ASSUMPTION: UDP does all the error checking. So data will be error free.
        if (l_ack_flg == '1') {
            if (l_ack_no - 1 == l_waiting_seq_no) {
                window_count--;
                // LOGIC FOR LOWEST WAITING SEQNO
                // GOTO NEXT_PACKET
                //
            } else {
                // GRAB ANOTHER PACKET
                // GOTO RECEIVE BLOCKING CALL
            }
        } else {
            // THINK
        }
        break;
    }
    // SEND A BLANK PACKET TO INDICATE THE END OF FILE.

    std::free(file);
    return;
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
    // p_packet = (pkt *)calloc()
    p_packet->seq_num = 0;
    p_packet->ackno = 0;
    p_packet->ack_flg = '0';
    // int load_size = (PACKET_SIZE - ((2*sizeof(char)) + (2*sizeof(unsigned int) + sizeof(unsigned
    // short))));
    int load_size = PACKET_SIZE;
    p_packet->load = (char *)calloc(load_size, sizeof(char));
    memset(p_packet->load, 0, PACKET_SIZE);
}
// -----------------------------------------------------------------------
void parse_packet(char *p_receive_buffer, pkt *p_packet) {
    p_packet->ack_flg = p_receive_buffer[1];
    receive_window = char_to_short(p_receive_buffer, 2);
    p_packet->seq_num = char_to_int(p_receive_buffer, 4);
    p_packet->ackno = char_to_int(p_receive_buffer, 8);
    memcpy(p_packet->load, p_receive_buffer + 12, PACKET_SIZE);

    // // int load_size = (PACKET_SIZE - (sizeof(char) + sizeof(unsigned int)));
    // p_packet->ack_flg = p_receive_buffer[0];
    // //p_packet->seq_num = chartoint(p_receive_buffer, 1);
    // p_packet->load = (char *)calloc(load_size, sizeof(char));
    // std::cout << "size of load --> " << load_size << std::endl;
    // memset(p_packet->load, 0, load_size);
    // memcpy(p_packet->load, p_receive_buffer + 5, load_size);
    // std::cout << "p_packet->seq_num --> " << p_packet->seq_num << std::endl;
}
