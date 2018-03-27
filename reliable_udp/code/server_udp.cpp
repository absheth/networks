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
#include <chrono>
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

enum state { SLOW_START, CONGESTION_AVOID, FAST_RECOVERY };

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
void calc_timeout(long sampleRTT, long long &estimatedRTT, int &deviationRTT,
                  long long &timeout_value);
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
int advertised_window;
int ip_port;
// -----------------------------------------------------------------------
// MAIN_METHOD

int main(int argc, char const *argv[]) {
    std::cout << "AKASH SHETH - SERVER" << std::endl;
    std::cout << "argc --> " << argc << '\n';
    if(argc != 3) {
        std::cout << "USAGE: ./server_udp port advertised_window_in_bytes" << std::endl;
        exit(0);
    }
    ip_port = atoi(argv[1]);
    std::cout << "PORT --> " << ip_port << std::endl;
    
    int x = atoi(argv[2])/DATA_SIZE;
    if (x == 0) {
        advertised_window = 1;
    } else {
        advertised_window = x+1;
    }
    
    std::cout << "WINDOW --> " << advertised_window << std::endl;
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
        // while(1) {
        //     sleep(5);
        //     std::cout << "STUCK HERE" << std::endl;
        //
        // }
        service_request(listen_socket);
        std::cout << "------------------------------" << std::endl;
        std::cout << ".. REQUEST SERVED .." << std::endl;
        std::cout << ".. COUNT --> " << count++ << '\n';
        std::cout << "------------------------------" << std::endl;
    }
    return 0;
}

// -----------------------------------------------------------------------
//
void service_request(int p_listen_socket) {
    // VARIABLE_DECLARATIONS OR INITIALIZATION
    int request_error = 0;
    char receive_buffer[PACKET_SIZE];
    int bytes_received = 0;
    unsigned int start_index = 0;

    // int total_packets_sent = 0;
    unsigned int base = 0;    // Last acknowledged packet/sequence number.
                              // server_waiting_on
    unsigned int nextseqnum;  // Increment when new sending new packet.
    auto start = std::chrono::high_resolution_clock::now();
    auto finish = std::chrono::high_resolution_clock::now();
    int congestion_window = 1;
    int sending_window = 0;
    // -----------------------------
LISTEN_AGAIN:

    memset(receive_buffer, 0, PACKET_SIZE);
    std::cout << std::endl;
    std::cout << "SERVING NOW.. Waiting for the request. " << std::endl;
    bytes_received = recvfrom(p_listen_socket, receive_buffer, PACKET_SIZE, 0,
                              (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen);
    std::cout << std::endl;
    std::cout << "BYTES READ --> " << bytes_received << std::endl;

    if (bytes_received < 0) {
        std::cout << std::endl;
        std::cout << "JUMP :: LISTEN_AGAIN" << std::endl;  // LISTEN_AGAIN
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
    FILE *file_descriptor;
    char *file;
    long fsize;
    char packet_padding;
    char packet_ack_flg;
    // unsigned int receive_window;
    unsigned int packet_seq_no;
    unsigned int packet_ack_no;
    int total_packets;
    char *packet;
    int sendto_value;
    unsigned int send_index;
    int sendsize;
    int datasize;
    long long timeout_value;
    char time_calc_flg;
    long sampleRTT;
    long long estimatedRTT;
    int deviationRTT;
    int phase;
    state current_state;
    unsigned int send_from;
    int from_timeout;
    int ssthresh;
    int duplicate_ack;
    int slowstart;
    int congestion;
    int fastrecovery;
    int last_packet;
    std::cout << std::endl;
    std::cout << "REQUEST :: VALID" << std::endl;
    std::cout << "REQUEST :: file --> " << receive_buffer + HEADER_SIZE << std::endl;
    if (!request_error) {
        file_descriptor = fopen(receive_buffer + HEADER_SIZE, "rb");
        if (file_descriptor == NULL) {
            total_packets = 1;
            packet_ack_flg = 'n';
            base = 0;
            congestion_window = 2;
            // std::cout << " TOTAL PACKETS --> " << total_packets<< std::endl;
            // std::cout << " PACKET ACK FLAG --> " << packet_ack_flg << std::endl;
            // std::cout << " BASE --> "  << base << std::endl;
            //
            std::cout << "########  FILE NOT FOUND  ########" << std::endl;
            goto SEND_DATA_PACKET;
        }
        fseek(file_descriptor, 0, SEEK_END);
        fsize = ftell(file_descriptor);
        fseek(file_descriptor, 0, SEEK_SET);
        file = (char *)malloc(fsize + 1);
        fread(file, fsize, 1, file_descriptor);
        fclose(file_descriptor);
        file[fsize] = 0;
        memset(receive_buffer, 0, PACKET_SIZE);

        // DEBUG
        /*
        std::cout << std::endl;
        std::cout << "*********************************************" << std::endl;
        std::cout << "FILE LOADED :: DETAILS" << std::endl;
        std::cout << "FILE :: fsize --> " << fsize << std::endl;
        std::cout << "FILE --> " << file << std::endl;

        std::cout << "*********************************************" << std::endl;
        std::cout << std::endl;
        */

        // INITIALIZATION

        base = start_index;
        nextseqnum = start_index;
        // advertised_window = 9;  // Make it dynamic. Command-line arugment
        packet_padding = '-';
        packet_ack_flg = 'p';
        packet_seq_no = base;
        packet_ack_no = base;
        total_packets = fsize / DATA_SIZE + ((fsize % DATA_SIZE == 0 ? 0 : 1));
        sendto_value = 0;
        send_index = 0;
        sendsize = 0;
        datasize = 0;
        timeout_value = 2000000;
        time_calc_flg = 'n';
        estimatedRTT = 0;
        deviationRTT = 1;
        last_packet = 0;
        current_state = SLOW_START;
        from_timeout = 0;
        ssthresh = 64000 / DATA_SIZE;
        duplicate_ack = 0;
        slowstart = 1;
        congestion = 0;
        fastrecovery = 0;
        std::cout << "Advertised window --> " << advertised_window << std::endl;
        std::cout << "TOTAL PACKETS --> " << total_packets << std::endl;

        std::cout << std::endl;
        std::cout << "~~~~~~~~ FILE SENDING :: START ~~~~~~~~" << std::endl;
        std::cout << std::endl;
        std::cout << "-- INITIAL DETAILS --" << std::endl;

        std::cout << "INITIAL :: packet_padding     --> " << packet_padding << std::endl;
        std::cout << "INITIAL :: packet_ack_flg     --> " << packet_ack_flg << std::endl;
        std::cout << "INITIAL :: advertised_window  --> " << advertised_window << std::endl;
        std::cout << "INITIAL :: packet_seq_no      --> " << packet_seq_no << std::endl;
        std::cout << "INITIAL :: packet_ack_no      --> " << packet_ack_no << std::endl;

        std::cout << "-- INITIAL DETAILS --" << std::endl;
        std::cout << std::endl;

        // std::cout << std::endl;
        // nextseqnum = (total_packets / advertised_window != 0 ? advertised_window
        //                                                      : total_packets %
        //                                                      advertised_window);

        while (1) {
        OUTER:
            if (last_packet == total_packets) {
                // SET TO INITIAL VALUES FOR SERVING AGAIN.
                //
                base = 0;
                nextseqnum = 0;
                send_index = 0;
                sendsize = 0;
                sendto_value = 0;
                time_calc_flg = 'n';
                if (packet_ack_flg != 'n') {
                    std::free(file);
                }
                std::cout << std::endl;
                
                std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
                std::cout << "Slow start count --> " << slowstart << std::endl;
                std::cout << "Congestion avoidance count --> " << congestion << std::endl;
                std::cout << "Fast recovery count --> " << fastrecovery << std::endl;
                
                std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
                
                std::cout << std::endl;

                break;
            }

        SEND_DATA_PACKET:

            int send_count = 0;
            // calculate sending window
            sending_window = congestion_window;
            // SENDING INDEX CHANGE
            while (send_count < sending_window) {
                if (from_timeout) {
                    send_index = base + send_count++;
                } else {
                    send_index = nextseqnum + send_count++;
                }
                if (send_index == total_packets) {
                    std::cout << std::endl;
                    std::cout << "###### ALL PACKETS SENT ######" << std::endl;

                    // base = send_index;
                    std::cout << "<< NOT SENDING ANYMORE >>" << std::endl;
                    if (packet_ack_flg == 'n') {
                        last_packet = total_packets;
                        goto OUTER;
                    }
                    std::cout << "JUMP TO :: RECEIVE " << std::endl;
                    //  std::cout << "###### ALL PACKETS SENT ######" << std::endl;
                    goto RECEIVE;
                }
                // std::cout << "SENDING DATA FOR PACKET --> " << send_index << std::endl;

                packet = (char *)calloc(PACKET_SIZE, sizeof(char));
                memset(packet, 0, PACKET_SIZE);
                // LAST_PACKET
                sendsize = PACKET_SIZE;
                if (packet_ack_flg != 'n') {
                    sendsize = PACKET_SIZE;
                    datasize = DATA_SIZE;
                    if (send_index == (total_packets - 1)) {
                        packet_ack_flg = 'x';
                        datasize = (fsize - send_index * DATA_SIZE);
                        sendsize = datasize + HEADER_SIZE;
                    }

                    packet[0] = packet_padding;
                    packet[1] = packet_ack_flg;
                    number_to_char(packet, advertised_window, send_index, packet_ack_no, 2);
                    memcpy(packet + HEADER_SIZE, file + (DATA_SIZE * send_index), datasize);
                } else {
                    packet[1] = packet_ack_flg;
                    number_to_char(packet, advertised_window, send_index, packet_ack_no, 2);
                }
                // std::cout << "SENDING --> " << (packet+HEADER_SIZE) << std::endl;
                if (time_calc_flg == 'n' && packet_ack_flg != 'n') {
                    if (send_count == sending_window - 1 && packet_ack_flg != 'x') {
                        start = std::chrono::high_resolution_clock::now();
                        // std::cout << " SETTING TIMER ON PACKET --> " << send_index << std::endl;
                        time_calc_flg = 'y';
                    }
                }
                sendto_value = sendto(p_listen_socket, packet, sendsize, 0,
                                      (struct sockaddr *)&clientaddr, clientlen);
                // std::cout << "SENDTO VALUE --> " << sendto_value << std::endl;
                if (sendto_value < 0) {
                    // std::cout << "SENDING FAILED :: Packet --> " << send_index << std::endl;
                    perror("PACKET SENDING FAILED");
                }

                std::free(packet);
                // exit(0);
                std::cout << "PACKET SENT :: Packet --> " << send_index << std::endl;
            }

            nextseqnum = base + send_count;
            std::cout << "AFTER SENDING :: nextseqnum --> " << nextseqnum << std::endl;
            std::cout << std::endl;

        RECEIVE:
            std::cout << "#########################################################################"
                      << std::endl;
            std::cout << " -- " << std::endl;

            std::cout << "CURRENT STATE --> " << current_state << std::endl;

            std::cout << " -- " << std::endl;

            std::cout << std::endl;
            std::cout << "** WAITING FOR ACK --> " << base << std::endl;
            memset(receive_buffer, 0, PACKET_SIZE);
            int rval;
            fd_set fds;
            struct timeval tv;
            if (timeout_value < 999999) {
                tv.tv_sec = 0;
                tv.tv_usec = timeout_value;
            } else {
                tv.tv_sec = timeout_value / 1000000;
                tv.tv_usec = timeout_value % 1000000;
            }
            // tv.tv_sec = 0;
            // tv.tv_usec = 999999;
            // std::cout << "TIMER ::  secs --> " << tv.tv_sec << " | usec --> " << tv.tv_usec
            //           << std::endl;
            // std::cout << std::endl;

            FD_ZERO(&fds);
            FD_SET(p_listen_socket, &fds);
            rval = select(p_listen_socket + 1, &fds, NULL, NULL, &tv);
            // std::cout << "RVAL FROM SELECT --> " << rval << std::endl;

            if (rval < 0) {
                perror("~~~~~ TIMER :: ERROR IN SELECT ~~~~~");
                exit(0);
            } else if (rval == 0) {
                slowstart++;
                std::cout << "SERVER ENTERING -- SLOW START" << std::endl;
                current_state = SLOW_START;
                ssthresh = congestion_window / 2;
                congestion_window = 1;
               duplicate_ack = 0; 
                // TIMEOUT
                //
                // ALWAYS SEND PACKETS ON TIME OUT
                // std::cout << std::endl;
                // std::cout << "***** TIMER :: TIMEOUT *****" << std::endl;
                // std::cout << "JUMPING TO --> SEND_DATA_PACKET " << std::endl;
                // std::cout << std::endl;
                packet_ack_flg = 'p';
                time_calc_flg = 'n';
                // std::cout << "setting packet_ack_flg --> " << packet_ack_flg << std::endl;
                // std::cout << std::endl;
                std::cout << "----------------FROM TIMEOUT----------------" << std::endl;

                from_timeout = 1;
                goto SEND_DATA_PACKET;
            } else {
                // Data available to read. READ IT.
                //
                bytes_received = recvfrom(p_listen_socket, receive_buffer, PACKET_SIZE, 0,
                                          (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen);
            }
            // TIMER
            if (bytes_received < FAILURE) {
                perror("RECEIVE :: ERROR");
                // exit(-1);
            }

            // EXTRACTION
            packet_padding = receive_buffer[0];
            packet_ack_flg = receive_buffer[1];
            // advertised_window = char_to_short(receive_buffer, 2);
            packet_seq_no = char_to_int(receive_buffer, 4);
            packet_ack_no = char_to_int(receive_buffer, 8);

            std::cout << std::endl;
            std::cout << "******************************************************" << std::endl;
            //     std::cout << "" << std::endl;

            std::cout << "TOTAL PACKETS --> " << total_packets << std::endl;
            //     // std::cout << "RECEIVED :: bytes_received --> " << bytes_received << std::endl;
            //     // std::cout << "RECEIVED :: packet_padding --> " << packet_padding << std::endl;
            //     std::cout << "RECEIVED :: packet_ack_flg --> " << packet_ack_flg << std::endl;
            //     // std::cout << "RECEIVED :: advertised_window --> " << advertised_window <<
            //     std::endl; std::cout << "RECEIVED :: packet_seq_no --> " << packet_seq_no <<
            //     std::endl;
            std::cout << "RECEIVED :: packet_ack_no --> " << packet_ack_no << std::endl;

            std::cout << std::endl;
            if (packet_ack_flg == 'x') {
                // ALL PACKETS ACKNOWLEDGEMENT
                // std::cout << "ACK FOR LAST PACKET --> " << packet_ack_no << std::endl;
                last_packet = total_packets;
                base = packet_seq_no;
                //      // nextseqnum = packet_ack_no;
                //      std::cout << std::endl;
                //      std::cout << "----------------------" << std::endl;
                //      std::cout << "UPDATED :: base --> " << base << std::endl;
                //      // std::cout << "UPDATED :: nextseqnum --> " << nextseqnum << std::endl;
                //      std::cout << "----------------------" << std::endl;
                //      std::cout << std::endl;
                //      std::cout << "-------- ALL PACKETS ACKNOWLEDGED ------- " << std::endl;
                //      std::cout << "JUMP TO :: OUTER" << std::endl;
                //      std::cout << "******************************************************" <<
                //      std::endl;

                goto OUTER;
            }
            // if (packet_ack_flg != '') {
            if (packet_ack_no == base - 1) {
                std::cout << std::endl;
                std::cout << std::endl;
                std::cout << std::endl;
                std::cout << std::endl;
                std::cout << "DUCPLICATE" << std::endl;
                duplicate_ack++;
                std::cout << std::endl;
                std::cout << "packet_ack_no --> " << packet_ack_no << std::endl;
                std::cout << "base --> " << base << std::endl;

                std::cout << std::endl;
                std::cout << std::endl;
                std::cout << std::endl;
                std::cout << std::endl;
                std::cout << std::endl;

                
                std::cout << "DUP ACKS --> " << duplicate_ack << std::endl;
                if(current_state == FAST_RECOVERY) {
                    congestion_window += congestion_window; 
                   packet_ack_no = base;
                } 
                if (duplicate_ack == 3) {
                    base = packet_ack_no;
                    from_timeout = 1;
                    fastrecovery++;
                    current_state = FAST_RECOVERY;
                    std::cout << "SERVER ENTERING -- FAST_RECOVERY " << std::endl;
                    // current_state = SLOW_START;
                    ssthresh = congestion_window / 2;
                    congestion_window = ssthresh + 3; // NOT SURE
                    // TIMEOUT
                    //
                    // ALWAYS SEND PACKETS ON TIME OUT
                    // std::cout << std::endl;
                    // std::cout << "***** TIMER :: TIMEOUT *****" << std::endl;
                    // std::cout << "JUMPING TO --> SEND_DATA_PACKET " << std::endl;
                    // std::cout << std::endl;
                    packet_ack_flg = 'p';
                    time_calc_flg = 'n';
                    // std::cout << "setting packet_ack_flg --> " << packet_ack_flg << std::endl;
                    // std::cout << std::endl;
                    // std::cout << "----------------FROM TIMEOUT----------------" << std::endl;
                    
                    goto SEND_DATA_PACKET;
                }
                // goto RECEIVE;
            }

            // }
            if (packet_ack_no == 0 && packet_ack_flg != 's') {
                from_timeout = 1;
                base = 1;
                packet_ack_flg = 'a';
                // nextseqnum = base;
                // packet_seq_no = base;
                goto SEND_DATA_PACKET;
            }
            if (packet_ack_no >= base) {
                base = packet_ack_no + 1;
                nextseqnum += base - packet_ack_no;
                //  if (packet_ack_no == 0) {
                //      nextseqnum += packet_ack_no + 1;
                //  } else {
                //      nextseqnum += packet_ack_no + 1 - base;
                //  }

                //   // if (congestion_window < advertised_window) {
                //   std::cout << std::endl;
                //   std::cout << "========================================" << std::endl;
                //   std::cout << "INITIAL CONGESTION WINDOW --> " << congestion_window <<
                //   std::endl; congestion_window += std::min(2 * advertised_window,
                //   congestion_window); std::cout << "----------------------------------------" <<
                //   std::endl; std::cout << "FINAL CONGESTION WINDOW --> " << congestion_window <<
                //   std::endl; std::cout << "========================================" <<
                //   std::endl; std::cout << std::endl;
                //   //}
                std::cout << "ssthresh --> " << ssthresh << std::endl;

                if (current_state == SLOW_START) {
                    duplicate_ack = 0;
                    if (congestion_window >= ssthresh) {
                        std::cout << "SERVER ENTERING -- CONGESTION AVOIDANCE." << std::endl;
                        std::cout << std::endl;
                        congestion++;
                        current_state = CONGESTION_AVOID;
                    } else {
                        slowstart++;
                        std::cout << std::endl;
                        std::cout << "========================================" << std::endl;
                        std::cout << "INITIAL CONGESTION WINDOW --> " << congestion_window
                                  << std::endl;
                        congestion_window += std::min(2 * advertised_window, congestion_window);
                        std::cout << "----------------------------------------" << std::endl;
                        std::cout << "FINAL CONGESTION WINDOW --> " << congestion_window
                                  << std::endl;
                        std::cout << "========================================" << std::endl;
                        std::cout << std::endl;
                    }
                }

                if (current_state == CONGESTION_AVOID) {
                    duplicate_ack = 0;
                    congestion_window += 1;
                }
                if (current_state == FAST_RECOVERY) {
                    std::cout << "ENTERING CONGESTION AVOIDANCE " << std::endl;
                    congestion++; 
                    duplicate_ack = 0; 
                    current_state = CONGESTION_AVOID;
                    congestion_window = ssthresh;
                }
                // nextseqnum += packet_ack_no - nextseqnum;
                // nextseqnum += base;
                if (time_calc_flg == 'y') {
                    // calculate estimated time

                    // std::cout << "STOPPING TIMER ON PACKET --> " << packet_ack_no << std::endl;
                    finish = std::chrono::high_resolution_clock::now();
                    long long elapsed_micro =
                        std::chrono::duration_cast<std::chrono::microseconds>(finish - start)
                            .count();
                    // std::cout << "elapsed_micro --> " << elapsed_micro << std::endl;
                    if (estimatedRTT == 0) {
                        estimatedRTT = elapsed_micro;
                    }
                    calc_timeout(elapsed_micro, estimatedRTT, deviationRTT, timeout_value);
                    // std::cout << "NEW TIME --> " << timeout_value << std::endl;
                    time_calc_flg = 'n';
                }
                std::cout << std::endl;
                std::cout << "----------------------" << std::endl;
                std::cout << "UPDATED :: base --> " << base << std::endl;
                std::cout << "UPDATED :: nextseqnum --> " << nextseqnum << std::endl;
                std::cout << "----------------------" << std::endl;
                std::cout << std::endl;
                from_timeout = 0;

                std::cout << "----------------------FROM VALID ACK----------------------"
                          << std::endl;
                if (nextseqnum < total_packets) {
                    goto SEND_DATA_PACKET;
                } else {
                    goto RECEIVE;
                }
            } else {
                std::cout << "ACK ALREADY RECEIVED :: packet_ack_no --> " << packet_ack_no
                          << std::endl;

                std::cout << "" << std::endl;

                goto RECEIVE;
            }
            std::cout << "******************************************************" << std::endl;
        }  // END_MAIN_WHILE
    }
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
    server.sin_port = htons(ip_port);
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

void calc_timeout(long sampleRTT, long long &estimatedRTT, int &deviationRTT,
                  long long &timeout_value) {
    std::cout << "estimatedRTT --> " << estimatedRTT << std::endl;
    std::cout << "deviationRTT --> " << deviationRTT << std::endl;
    std::cout << "timeout_value  --> " << timeout_value << std::endl;
    if (sampleRTT != estimatedRTT) {
        sampleRTT -= (estimatedRTT >> 3);
    }
    estimatedRTT += sampleRTT;
    if (sampleRTT < 0) {
        sampleRTT = -sampleRTT;
    }
    sampleRTT -= (deviationRTT >> 3);
    deviationRTT += sampleRTT;
    timeout_value = (estimatedRTT >> 3) + (deviationRTT >> 1);
    std::cout << "NEW TIMEOUT --> " << timeout_value << std::endl;
}
