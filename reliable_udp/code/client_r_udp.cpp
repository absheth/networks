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
#include <iostream>
#include <list>
#include <sstream>

#define SUCCESS 0
#define FAILURE -1
#define MAXBUFFERSIZE 1000000000  // 999999999
#define PACKET_SIZE 1472
#define DATA_SIZE 1460
#define HEADER_SIZE 12
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
unsigned int total_received = 0;
unsigned short receive_window = 0;
unsigned int current_ackno = 0;
char padding = '-';
int window_size = 0;
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// LIST_FUNCTIONS

int get_element_index(std::list<unsigned int> &p_list, unsigned int p_element);
unsigned int get_min_element(std::list<unsigned int> &p_list);
unsigned int get_max_element(std::list<unsigned int> &p_list);
int erase_element(std::list<unsigned int> *p_list, unsigned int p_element);
void print_list(std::list<unsigned int> &p_list);

std::list<unsigned int> ack_send_list;

// -----------------------------------------------------------------------
char *main_data;
// -----------------------------------------------------------------------
int main(int argc, char const *argv[]) {
    std::cout << "AKASH SHETH - CLIENT" << std::endl;
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
    pkt rqst_packet;
    init_packet(&rqst_packet);
    total_received = rqst_packet.seq_num;

    rqst_packet.load[0] = padding;
    rqst_packet.load[1] = rqst_packet.ack_flg;
    rqst_packet.seq_num = 0;
    rqst_packet.ackno = 0;
    receive_window = 65535;
    number_to_char(rqst_packet.load, receive_window, rqst_packet.seq_num, rqst_packet.ackno, 2);
    // std::string filename = "server_r_udp.cpp";
    std::string filename = "1mb.txt";
    memcpy(rqst_packet.load + 12, filename.c_str(), filename.length());
    /*
     * DEBUG
    unsigned short l_rwin = char_to_short(rqst_packet.load, 2);
    std::cout << "l_rwin--> " << l_rwin << std::endl;

    unsigned int l_seq = char_to_int(rqst_packet.load, 4);
    std::cout << "l_seq--> " << l_seq << std::endl;

    unsigned int l_ack = char_to_int(rqst_packet.load, 8);
    std::cout << "l_ack--> " << l_ack << std::endl;
    */
    //***************************************************
    auto start = std::chrono::high_resolution_clock::now();
    if (sendto(socketFD, rqst_packet.load, PACKET_SIZE, 0, (struct sockaddr *)&serveraddr,
               serverlen) < 0) {
        std::free(rqst_packet.load);
        perror("sending error");
        return -1;
    }
    std::free(rqst_packet.load);
    // return 0;
    // int size = 0;
    std::cout << "------------ RESPONSE START ------------" << '\n';
    window_size = 9;
    std::cout << "WINDOW SIZE --> " << window_size << std::endl;

    main_data = (char *)calloc(MAXBUFFERSIZE, sizeof(char));
    char receive_buffer[PACKET_SIZE];
    unsigned int l_seq_no = 0;
    unsigned int l_ack_no = 0;
    char l_ack_flg = '1';
    unsigned int waiting_seq_no = 0;
    int already_received = 0;
    int waiting_ack_count = 0;
    // int window_count = 0;
    // int isWaitingSet = 0;
    // int sendfromlast = 0;  // 0 means from l_seq_no  || 1 means from total_sent
    while (1) {
    MAIN:
        if (padding == 'x' && l_ack_no == waiting_seq_no ) {
            break;
        }
    RECEIVE:
        while (ack_send_list.size() < window_size) {
            memset(receive_buffer, 0, PACKET_SIZE);

            // TIMER
            int rval;
            fd_set fds;
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 500000 ;
            std::cout << "TIMER TIMEOUT ::  secs --> " << tv.tv_sec << " | usec --> " << tv.tv_usec
                      << std::endl;
            std::cout << std::endl;
            
            FD_ZERO(&fds);
            FD_SET(socketFD, &fds);
            rval = select(socketFD + 1, &fds, NULL, NULL, &tv);
            std::cout << "RVAL --> " << rval << std::endl;

            if (rval < 0) {
                perror("~~~~~ ERROR WHILE SELECTING ~~~~~");

            } else if (rval == 0) {
                // TIMEOUT
                std::cout << std::endl;
                std::cout << "***** TIMEOUT *****" << std::endl;
                std::cout << "JUMPING TO --> SEND_ACK " << std::endl;
                std::cout << "***** TIMEOUT *****" << std::endl;
                std::cout << std::endl;
                
                goto SEND_ACK;
            } else {
                // Data available to read. READ IT.
                if ((received = recvfrom(socketFD, receive_buffer, PACKET_SIZE, 0,
                                         (struct sockaddr *)&serveraddr,
                                         (socklen_t *)&serverlen)) == FAILURE) {
                    perror("receive error");
                    // exit(-1);
                }
            }
            std::cout << std::endl;
            std::cout << "************************************************************************"
                      << std::endl;

            std::cout << "RECEIVED BYTES --> " << received << std::endl;
            l_seq_no = char_to_int(receive_buffer, 4);
            std::cout << "RECEIVED SEQUENCE --> " << l_seq_no << std::endl;
            l_ack_no = char_to_int(receive_buffer, 8);
            std::cout << "RECEIVED ACKNOWLEDGEMENT --> " << l_ack_no << std::endl;
            std::cout << "R:: waiting_seq_no --> " << waiting_seq_no << std::endl;
            std::cout << "R:: TOTAL RECEIVED --> " << total_received << std::endl;
            waiting_ack_count = receive_buffer[1] - '0';
            std::cout << "WAITING FOR ACK ON SERVER SIDE: " << waiting_ack_count << std::endl;
            if(padding == 'x' && l_ack_no == waiting_seq_no ) {
                goto MAIN;
            }
            if (receive_buffer[0] == 'x') {
                padding = 'x';
            }
            std::cout << "************************************************************************"
                      << std::endl;
            std::cout << std::endl;
            if (get_element_index(ack_send_list, l_seq_no) == ack_send_list.size()) {
                // RECEIVING FOR THE FIRST TIME or AGAIN. Add to the list of ACKS to be sent.
                std::cout << "INSERTING IN THE LIST :: l_seq_no --> " << l_seq_no << std::endl;
                std::cout << std::endl;
                 
                ack_send_list.push_back(l_seq_no);
            } else {
                // RECEIVED. READ NEXT PACKET.
                // goto RECEIVE;
                std::cout << "ALREADT RECEIVED :: packet with l_seq_no --> " << l_seq_no
                          << std::endl;
                std::cout << "Continuing to get another packet.. " << std::endl;

                continue;
            }
            int received_data_size = received - HEADER_SIZE;
            
            std::cout << std::endl;
            std::cout << std::endl;
            std::cout << "=====================================================" << std::endl;
            std::cout << "OLD TOTAL RECEIVED --> " << total_received << std::endl;
            std::cout << "RECEIVED DATA SIZE --> " << received_data_size << std::endl;
            
            memcpy(main_data + total_received, receive_buffer + 12, received_data_size);
            total_received += received_data_size;
            std::cout << "NEW TOTAL RECEIVED --> " << total_received << std::endl;
            
            std::cout << "=====================================================" << std::endl;
            std::cout << std::endl;
            std::cout << std::endl;
            

            if (l_seq_no == waiting_seq_no) {
                std::cout << "l_seq_no == waiting_seq_no -->  " << l_seq_no
                          << " == " << waiting_seq_no << std::endl;
                std::cout << "REMOVE IT FROM THE LIST" << std::endl;
                std::cout << "JUMPING TO --> REMOVE " << std::endl;

                goto REMOVE;
            }
        }
    REMOVE:

        if (l_seq_no == waiting_seq_no) {
            int removed_index = erase_element(&ack_send_list, waiting_seq_no);
            if (removed_index > ack_send_list.size()) {
                // DELETION FAILED
                std::cout << "NOT PRESENT IN LIST " << waiting_seq_no << std::endl;
                perror("NOT PRESENT IN LIST");
            } else {
                std::cout << "waiting_seq_no --> " << waiting_seq_no
                          << " deleted from the list on index --> " << removed_index << std::endl;
                if (padding != 'x') {
                    // IF LAST PACKET NOT ALREADY RECEIVED
                    waiting_seq_no += DATA_SIZE;
                } else {
                    // IF LAST PACKET ALREADY RECEIVED
                    waiting_seq_no = total_received - received;  // NOT SURE;
                }
            }

            if (get_element_index(ack_send_list, waiting_seq_no) != ack_send_list.size()) {
                std::cout << "waiting_seq_no --> " << waiting_seq_no
                          << "  :: ALREADY PRESENT IN THE LIST." << std::endl;
                std::cout << "JUMPING TO --> REMOVE" << std::endl;
                std::cout << std::endl;
                
                goto REMOVE;
            }
            /*
            if (ack_send_list.size() == 0) {

                std::cout << "JUMPING TO --> REMOVE" << std::endl;
                goto SEND_ACK;
            }
            */

            std::cout << "JUMPING TO --> RECEIVE :: END OF REMOVE" << std::endl;
            std::cout << std::endl;
             
            goto RECEIVE;
        }
    SEND_ACK:
        // std::cout << "---------------------------" << std::endl;
        // unsigned int sleep_time = 50;
        // usleep(sleep_time);
        // unsigned int sleep_time = 2;
        // std::cout << "SLEEPING FOR " << sleep_time << std::endl;
        // sleep(sleep_time);
        // std::cout << "---------------------------" << std::endl;
        // std::cout << std::endl;
        std::cout << "##################" << std::endl;
        std::cout << "SENDING ACK" << std::endl;
        std::cout << "SENDING RECEIVE WINDOW --> " << receive_window << std::endl;
        std::cout << "SENDING TOTAL RECEIVED --> " << total_received << std::endl;
        std::cout << "S:: waiting_seq_no --> " << waiting_seq_no << std::endl;
        std::cout << "ACK SEND LIST " << std::endl;
        print_list(ack_send_list);
        std::cout << "##################" << std::endl;
        std::cout << std::endl;
        pkt send_pkt;
        init_packet(&send_pkt);

        send_pkt.load[0] = padding;
        send_pkt.load[1] = '1';
        unsigned int pack_ack = 0;

        if (waiting_seq_no != 0) {
            pack_ack = total_received ;
        }

        if(padding == 'x' && ack_send_list.size() == 0) {
            // pack_ack = l_ack_no;
            pack_ack = total_received;
            waiting_seq_no = total_received;
        }
        
        number_to_char(send_pkt.load, receive_window, waiting_seq_no, pack_ack, 2);
        int sendto_value = sendto(socketFD, send_pkt.load, PACKET_SIZE, 0,
                                  (struct sockaddr *)&serveraddr, serverlen);

        std::cout << "SENDTO_VALUE --> " << sendto_value << std::endl;

        if (sendto_value < 0) {
            std::free(rqst_packet.load);
            perror("sending error");
            return -1;
        }

        // RESPONSE SENT. Free the load.
        std::free(send_pkt.load);
        memset(receive_buffer, 0, PACKET_SIZE);
        if (padding == 'x' && waiting_ack_count == 0) {
            break;
        }
        goto RECEIVE;
    }
    std::string final_data(main_data);
    std::free(main_data);
    std::cout << "Received data --> " << final_data << std::endl;

    close(socketFD);
    std::cout << "------------ RESPONSE END ------------" << '\n';
    std::cout << '\n';
    std::cout << "total received bytes--> " << total_received << '\n';
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
// -----------------------------------------------------------------------

// LIST_FUNCTIONS

// -----------------------------------------------------------------------
int get_element_index(std::list<unsigned int> &p_list, unsigned int p_element) {
    int index = 0;
    std::list<unsigned int>::iterator findIter = std::find(p_list.begin(), p_list.end(), p_element);
    index = std::distance(p_list.begin(), findIter);

    return index;
}
unsigned int get_min_element(std::list<unsigned int> &p_list) {
    // EMPTY LIST // RETURN -1
    if (p_list.size() == 0) {
        return -1;
    }
    unsigned int l_element = 0;
    std::list<unsigned int>::iterator iter = std::min_element(p_list.begin(), p_list.end());
    l_element = *iter;
    return l_element;
}

unsigned int get_max_element(std::list<unsigned int> &p_list) {
    // EMPTY LIST // RETURN -1
    if (p_list.size() == 0) {
        return -1;
    }

    unsigned int l_element = 0;
    std::list<unsigned int>::iterator iter = std::max_element(p_list.begin(), p_list.end());
    l_element = *iter;
    return l_element;
}

int erase_element(std::list<unsigned int> *p_list, unsigned int p_element) {
    int l_element_index = 0;
    l_element_index = get_element_index(*(p_list), p_element);
    if (l_element_index == (*p_list).size()) {
        return l_element_index + 2;
    }
    std::list<unsigned int>::iterator it = (*p_list).begin();
    int count = 1;
    while (count <= l_element_index) {
        it++;
        count++;
    }
    (*p_list).erase(it);
    return l_element_index;
}

void print_list(std::list<unsigned int> &p_list) {
    std::list<unsigned int>::iterator it = p_list.begin();
    while (it != p_list.end()) {
        std::cout << (*it) << " ";
        it++;
    }
    std::cout << std::endl;
}
// -----------------------------------------------------------------------
