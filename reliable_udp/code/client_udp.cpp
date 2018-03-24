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
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <climits>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
// -----------------------------------------------------------------------
// CONSTANTS
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

// -----------------------------------------------------------------------
// MAIN_METHOD
int main(int argc, char const *argv[]) {
    std::cout << "AKASH SHETH - CLIENT" << std::endl;
    std::cout << std::endl;
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
    // CONNECTION
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
    //

    // VARIABLE_DECLARATIONS
    unsigned int receive_window = 0;
    unsigned int window_size = 0;
    char file_request[PACKET_SIZE];
    char receive_buffer[PACKET_SIZE];
    int total_packets_received = 0;
    unsigned int packet_seq_no = 0;
    unsigned int packet_ack_no = 0;
    unsigned int client_waiting_on = 0;
    int packet_drop_count = 0;
    int bytes_received = 0;
    char packet_padding = '-';
    int last_pack_size = 0;
    long long total_received = 0;
    char packet_ack_flg = 'a';
    int write_result = SUCCESS;
    // NOT_TO_USE // DECIDE
    unsigned int waiting_seq_no = 0;
    int already_received = 0;
    int waiting_ack_count = 0;
    char* data;
    // -----------------------------
    // std::string filename = "server_r_udp.cpp";
    // std::string filename = "1mb.txt";
    std::string filename = "1234.txt";
    memset(file_request, 0, sizeof(file_request));
    file_request[0] = '-';
    file_request[1] = packet_ack_flg;
    receive_window = 65535;
    number_to_char(file_request, receive_window, 0, 0, 2);
    memcpy(file_request + 12, filename.c_str(), filename.length());
    std::cout << "--> REQUESTING FILE: " << filename << " <--" << std::endl;
    std::cout << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    int request_sendto_value = sendto(socketFD, file_request, sizeof(file_request), 0,
                                      (struct sockaddr *)&serveraddr, serverlen);
    std::cout << "REQUEST_SENDTO_VALUE --> " << request_sendto_value << std::endl;

    if (request_sendto_value < 0) {
        perror("sending error");
        return -1;
    }
    memset(file_request, 0, sizeof(file_request));

    // -----------------------------
    std::cout << "------------ RESPONSE START ------------" << '\n';
    window_size = 9;
    std::cout << "WINDOW SIZE --> " << window_size << std::endl;

    while (1) {
        memset(receive_buffer, 0, PACKET_SIZE);
        bytes_received = recvfrom(socketFD, receive_buffer, PACKET_SIZE, 0,
                                  (struct sockaddr *)&serveraddr, (socklen_t *)&serverlen);
         // std::cout << "RECEIVE BUFFER --> " << receive_buffer+12 << std::endl;
        
        if (bytes_received < 0) {
            std::cout << std::endl;
            std::cout << "##############" << std::endl;
            std::cout << "RECEIVE_ERROR :: bytes_received --> " << bytes_received << std::endl;
            std::cout << "##############" << std::endl;
            std::cout << std::endl;
            perror("RECEIVE_ERROR");
            // exit(-1);
        }
        packet_padding = receive_buffer[0];
        packet_ack_flg = receive_buffer[1];
        receive_window = char_to_short(receive_buffer, 2);
        packet_seq_no = char_to_int(receive_buffer, 4);
        packet_ack_no = char_to_int(receive_buffer, 8);
        // memset(receive_buffer, 0, PACKET_SIZE);
        std::cout << std::endl;
        std::cout << "************************************************************************"
                  << std::endl;

        std::cout << "RECEIVED :: received_bytes --> " << bytes_received << std::endl;
        std::cout << "RECEIVED :: packet_padding --> " << packet_padding << std::endl;
        std::cout << "RECEIVED :: packet_ack_flg --> " << packet_ack_flg << std::endl;
        std::cout << "RECEIVED :: receive_window --> " << receive_window << std::endl;
        std::cout << "RECEIVED :: packet_seq_no  --> " << packet_seq_no << std::endl;
        std::cout << "RECEIVED :: packet_ack_no  --> " << packet_ack_no << std::endl;

        std::cout << "************************************************************************"
                  << std::endl;
        std::cout << std::endl;

        if (client_waiting_on == 0 && packet_seq_no != 0) {
            std::cout << std::endl;
            std::cout << "FIRST PACKET NOT RECEIVED" << std::endl;
            std::cout << "DROPPING PACKET --> " << packet_seq_no << std::endl;
            std::cout << "WAITING FOR THE FIRST PACKET WITH SEQUENCE --> " << client_waiting_on
                      << std::endl;

            packet_drop_count++;
            std::cout << std::endl;
            continue;
        } else if (client_waiting_on == packet_seq_no) {
            std::cout << std::endl;
            std::cout << "RECEIVED WAITING PACKET --> " << client_waiting_on << std::endl;
            // WRITE TO FILE AND SEND ACK.
            // data = (char *)calloc((bytes_received-HEADER_SIZE), sizeof(char));
            // memset(data, 0, (bytes_received-HEADER_SIZE));
            // memcpy(data, receive_buffer+HEADER_SIZE, bytes_received-HEADER_SIZE);
            // std::cout << data << std::endl;
            // std::free(data);
            
            int write_result =
                write_data_to_file(receive_buffer, bytes_received - HEADER_SIZE, packet_seq_no);
            if (write_result < 0) {
                std::cout << "TEMPORARY :: EXITING THE PROGRAM" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (packet_ack_flg != 'x') {
                packet_ack_flg = 'a';
            }
            client_waiting_on = packet_seq_no+1;
            
            total_packets_received++;
            // exit(0);
        } else {
            std::cout << std::endl;
            std::cout << "PACKET OUT OF ORDER --> " << packet_seq_no << std::endl;
            std::cout << "DROPPING THE PACKET --> " << packet_seq_no << std::endl;
            total_packets_received++;
            packet_drop_count++;
        }

        char *send_ack_packet = (char *)calloc(PACKET_SIZE, sizeof(char));
        memset(send_ack_packet, 0, PACKET_SIZE);
        send_ack_packet[0] = packet_padding;
        send_ack_packet[1] = packet_ack_flg;
        number_to_char(send_ack_packet, receive_window, client_waiting_on, client_waiting_on, 2);
        int ack_sendto_value = sendto(socketFD, send_ack_packet, PACKET_SIZE, 0,
                                      (struct sockaddr *)&serveraddr, serverlen);
        std::cout << std::endl;
        std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                  << std::endl;
        std::cout << "AFTER SENDING :: ack_sendto_value --> " << ack_sendto_value << std::endl;
        std::cout << "AFTER SENDING :: padding          --> " << send_ack_packet[0] << std::endl;
        std::cout << "AFTER SENDING :: ack flg          --> " << send_ack_packet[1] << std::endl;
        std::cout << "AFTER SENDING :: receive_window   --> " << receive_window << std::endl;
        std::cout << "AFTER SENDING :: sequence number  --> " << client_waiting_on << std::endl;
        std::cout << "AFTER SENDING :: ack number       --> " << client_waiting_on << std::endl;
        std::cout << "AFTER SENDING :: client_waiting_on--> " << client_waiting_on << std::endl;
        std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                  << std::endl;
        std::cout << std::endl;

        // FREE_MEMORY
        std::free(send_ack_packet);
        if (packet_ack_flg == 'x') {
            last_pack_size = bytes_received - HEADER_SIZE;
            std::cout << "LAST PACK :: size --> " << last_pack_size << std::endl;
            break;
        }
    }
    close(socketFD);
    std::cout << "------------ RESPONSE END ------------" << '\n';
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";

    // FINAL_COMPUTATION
    total_received = ((client_waiting_on - 1) * DATA_SIZE) + last_pack_size;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "************ FINAL STATISTICS  ************ " << std::endl;
    std::cout << "Total bytes received: " << total_received << std::endl;
    std::cout << "Total packets received: " << total_packets_received << std::endl;
    std::cout << "Total packets dropped: " << packet_drop_count << std::endl;

    std::cout << "************ FINAL STATISTICS  ************ " << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "---------------------------- END ----------------------------" << '\n';
    //***************************************************
    return 0;
}

// -----------------------------------------------------------------------
// CONVERSION FUNCTIONS
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
    return ((unsigned char)p_charstream[p_offset + 0] << 8) |
           (unsigned char)p_charstream[p_offset + 1];
}

/* Convert unsigned into char array of 4 bytes */
void number_to_char(char *p_char, unsigned short p_short_num, unsigned int p_seqnum,
                    unsigned int p_ackno, int p_offset) {
    // converting receive_window
    // std::cout << "p_short_num --> " << p_short_num << std::endl;

    p_char[p_offset + 0] = (p_short_num >> 8) & 0xFF;
    p_char[p_offset + 1] = p_short_num & 0xFF;

    // converting seq_num
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

// WRITE_DATA_FUNCTION
int write_data_to_file(char *p_received_data, int p_data_length, unsigned int p_packet_number) {
    std::cout << std::endl;
    std::cout << "******************************************************" << std::endl;
    std::cout << " Writing data :: Length --> " << p_data_length << " | Packet --> "
              << p_packet_number << std::endl;
    std::cout << "******************************************************" << std::endl;
    std::cout << std::endl;

    char filename[] = "test.txt";
    int l_return = SUCCESS;
    std::ofstream outputfile;

    // creating, opening and wrinting/appending data into a file.
    outputfile.open(filename, std::fstream::out | std::fstream::app);

    // Simple error handling for file creating/opening for writing,
    // test if fail to open the file.
    if (outputfile.fail()) {
        std::cout << std::endl;
        std::cout << "Creating and opening file " << filename << " for writing " << std::endl;
        std::cout << "FILE OPEN & CREATE :: FAILURE" << std::endl;
        std::cout << std::endl;
        std::cout << "Possible errors: " << std::endl;
        std::cout << "1. The file does not exists." << std::endl;
        std::cout << "2. The path was not found." << std::endl;
        std::cout << std::endl;
        l_return = FAILURE;

    } else {
        std::cout << std::endl;
        std::cout << "FILE OPEN & CREATE :: SUCCESS" << std::endl;
        std::cout << "-- WRITE :: START --" << std::endl;
        // std::string x(p_received_data+12);
        // std::cout << "DATA --> " << std::endl;
        std::cout << p_received_data + 12 << std::endl;
        
        
        // outputfile << x;
        outputfile.write(p_received_data+12, p_data_length);
        std::cout << "-- WRITE :: END --" << std::endl;

        // close the output file.
        outputfile.close();

        if (outputfile.fail()) {
            std::cout << "CLOSE :: FAILURE " << std::endl;
            std::cout << std::endl;
            l_return = FAILURE;
        } else {
            std::cout << "CLOSE :: SUCCESS " << std::endl;
            std::cout << std::endl;
        }
    }
    return l_return;
}

// -----------------------------------------------------------------------
