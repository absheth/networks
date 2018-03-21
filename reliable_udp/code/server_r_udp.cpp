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
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <list>
#include <sstream>

// -----------------------------------------------------------------------

#define PACKET_SIZE 1472
#define DATA_SIZE 1460
#define HEADER_SIZE 12
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
unsigned int total_sent = 0;
unsigned short receive_window = 0;
unsigned int current_ackno = 0;
char padding = '-';
int window_size = 0;

// -----------------------------------------------------------------------
// LIST_FUNCTIONS

int get_element_index(std::list<unsigned int> &p_list, unsigned int p_element);
unsigned int get_min_element(std::list<unsigned int> &p_list);
unsigned int get_max_element(std::list<unsigned int> &p_list);
int erase_element(std::list<unsigned int> *p_list, unsigned int p_element);
void print_list(std::list<unsigned int> &p_list);

std::list<unsigned int> ack_waiting_list;
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
    std::cout << "AKASH SHETH - SERVER" << std::endl;
    std::cout << "argc --> " << argc << '\n';
    working_directory = getenv("PWD");
    std::cout << "working_directory --> " << working_directory << '\n';
    // window_size = atoi(argv[1]);
    window_size = 3;
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
    int sum = 0;
    std::cout << '\n';
    std::cout << "SERVING NOW.." << '\n';
    char receive_buffer[PACKET_SIZE];
    int received_bytes = 0;
    memset(receive_buffer, 0, PACKET_SIZE);
    received_bytes = recvfrom(connFD, receive_buffer, PACKET_SIZE, 0,
                              (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen);
    // std::cout << "received_bytes--> " << received_bytes << std::endl;
    pkt received_pkt;
    init_packet(&received_pkt);
    parse_packet(receive_buffer, &received_pkt);
    //  int seq_num = char_to_int(receive_buffer, 4);
    //  std::cout << "seq_num --> " << received_pkt.seq_num << std::endl;
    //  std::cout << "Request --> " << received_pkt.load << std::endl;
    //  std::cout << "Request length --> " << strlen(received_pkt.load) << std::endl;
    //  std::cout << "ackno --> " << received_pkt.ackno << std::endl;
    //  std::cout << "receive_window--> " << receive_window << std::endl;
    //  std::cout << "ackflg --> " << received_pkt.ack_flg << std::endl;
    // --------------------
    FILE *f = fopen(received_pkt.load, "rb");
    // FILE *f = fopen("1mb.txt", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *file = (char *)malloc(fsize + 1);
    fread(file, fsize, 1, f);
    fclose(f);
    std::free(received_pkt.load);  // FREE MEMORY
    // -------------------_
    file[fsize] = 0;
    // std::cout << "FILE--> " << file << std::endl;
    std::cout << "FILE SIZE --> " << fsize << std::endl;
    std::cout << "WINDOW SIZE --> " << window_size << std::endl;

    // unsigned int l_seq_no = received_pkt.seq_num;
    // unsigned int l_ack_no = received_pkt.ackno;

    unsigned int l_seq_no = 0;
    unsigned int l_ack_no = 0;
    char l_ack_flg = '0';
    unsigned int waiting_ack_no = 0;
    int window_count = 0;
    int isWaitingSet = 0;
    int sendfromlast = 0;  // 0 means from l_seq_no  || 1 means from total_sent
    int data_size = 0;
    int final_sent = 0;
    while (total_sent <= fsize) {
        // First, send packets equal to the window size
        if (total_sent == fsize) {
            std::cout << std::endl;
            std::cout << "****ALL DATA SENT****" << std::endl;
            // std::cout << "LIST SIZE --> " << ack_waiting_list.size() << std::endl;

            if (final_sent) {
                goto RECEIVE;
            } else {
                std::cout << "****************** BREAKING ****************" << std::endl;
                break;
            }
            std::cout << "****ALL DATA SENT****" << std::endl;
            std::cout << std::endl;
        }
    SEND_PACKET:
        // SEND_PACKET
        while (ack_waiting_list.size() < window_size) {
            pkt send_pkt;
            init_packet(&send_pkt);
            // send_pkt.seq_num = total_sent;
            // send_pkt.ackno = current_ackno;
            // send_pkt.ack_flg = '0';

            send_pkt.load[0] = padding;
            send_pkt.load[1] = '0';
            // number_to_char(send_pkt.load, receive_window, total_sent, current_ackno, 2);
            // memcpy(send_pkt.load + 12, file + total_sent, DATA_SIZE);

            unsigned int send_index;
            if (sendfromlast) {
                send_index = total_sent;
            } else {
                if (ack_waiting_list.size() > 0) {
                    send_index = get_min_element(ack_waiting_list);
                } else {
                    // THINK // CHECK
                    send_index = l_seq_no;  // MAY BE MIN FROM THE LIST
                }
            }
            if (fsize - total_sent < DATA_SIZE) {
                data_size = fsize - total_sent;
                final_sent = 1;
            } else {
                data_size = DATA_SIZE;
            }
            // FOR LAST PACKET 
            int packetsize = PACKET_SIZE;
            if (final_sent) {
                packetsize = data_size + HEADER_SIZE;
                send_pkt.load[0] = 'x';
            }
            
            // number_to_char(send_pkt.load, receive_window, total_sent, current_ackno, 2);
            number_to_char(send_pkt.load, receive_window, send_index, l_ack_no, 2);
            memcpy(send_pkt.load + 12, file + send_index, data_size);
            if (sendto(connFD, send_pkt.load, packetsize, 0, (struct sockaddr *)&clientaddr,
                       clientlen) < 0) {
                std::free(send_pkt.load);
                perror("sending error");
                goto SEND_PACKET;
            } else {
                std::free(send_pkt.load);
            }

            // ********************************************

            // ********************************************

            if (!isWaitingSet) {
                // waiting_ack_no = total_sent;
                waiting_ack_no = send_index;
                isWaitingSet = 1;
            }
             
            total_sent += data_size;
            sendfromlast = 1;
            ack_waiting_list.push_back(total_sent);
            std::cout << std::endl;
            std::cout << "##################" << std::endl;
            std::cout << "SEND SIZE --> " << data_size << std::endl;
            std::cout << "SENDING RECEIVE WINDOW --> " << receive_window << std::endl;
            std::cout << "SENDING TOTAL SENT --> " << total_sent << std::endl;
            std::cout << "S:: waiting_ack_no --> " << waiting_ack_no << std::endl;
            std::cout << "LIST SIZE --> " << ack_waiting_list.size() << std::endl;
            print_list(ack_waiting_list);
            std::cout << "##################" << std::endl;
            std::cout << std::endl;
        }
        // TIMER GOES SOMEWHERE HERE
        // char receive_buffer[PACKET_SIZE];
        int received_bytes;
    RECEIVE:
        unsigned int sleep_time = 50;
        std::cout << std::endl;
        std::cout << "---------------------------" << std::endl;
        std::cout << "SLEEPING FOR " << sleep_time << " MILLISECONDS " << std::endl;
        // usleep(sleep_time);
        std::cout << "---------------------------" << std::endl;
        std::cout << std::endl;
        memset(receive_buffer, 0, PACKET_SIZE);
        received_bytes = recvfrom(connFD, receive_buffer, PACKET_SIZE, 0,
                                  (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen);
        std::cout << std::endl;
        std::cout << "************************************************************************"
                  << std::endl;
        std::cout << "FILESIZE --> " << fsize << std::endl;

        std::cout << "RECEIVED BYTES --> " << received_bytes << std::endl;
        char lst_received = receive_buffer[0];
        std::cout << "PADDING --> " << lst_received << std::endl;
        l_ack_flg = receive_buffer[1];
        std::cout << "RECEIVED ACK_FLG --> " << l_ack_flg << std::endl;

        receive_window = char_to_short(receive_buffer, 2);
        std::cout << "RECEIVE WINDOW --> " << receive_window << std::endl;
        l_seq_no = char_to_int(receive_buffer, 4);
        std::cout << "RECEIVED SEQUENCE --> " << l_seq_no << std::endl;
        l_ack_no = char_to_int(receive_buffer, 8);
        std::cout << "RECEIVED ACKNOWLEDGEMENT --> " << l_ack_no << std::endl;
        std::cout << "R:: waiting_ack_no --> " << waiting_ack_no << std::endl;
        std::cout << "ACK WAITING LIST" << std::endl;
        print_list(ack_waiting_list);
        std::cout << "************************************************************************"
                  << std::endl;
        std::cout << std::endl;
        if (lst_received == 'x') {
            break;
        }
        // GOTO RECEIVE BLOCKING CALL : recvfrom();

        // CHeck for the ack flag .
        // Suppose you receive a packet with an ack = 1 and service_request
        // ASSUMPTION: UDP does all the error checking. So data will be error free.
        // NOTE: ACK FLAG WILL ALWAYS BE 1.
        if (l_ack_flg == '1') {
            if (l_ack_no - 1 == waiting_ack_no) {
                // ACKNOWLEDGEMENT RECEIVED SUCCESSFULLY. REMOVE IT FROM ack_waiting_list.
                //  int removed_index = erase_element(&ack_waiting_list, waiting_ack_no);
                //  if (removed_index > ack_waiting_list.size()) {
                //      // DELETION FAILED.
                //      // perror("COULD NOT DELETE SEQUENCE NUMBER");
                //      quit("COULD NOT DELETE SEQUENCE NUMBER");
                //  } else {
                //      std::cout << "ELEMENT REMOVED FROM --> " << removed_index << std::endl;
                //  }

                // LOGIC FOR LOWEST WAITING SEQNO
                if (ack_waiting_list.size() > 0) {
                    waiting_ack_no = get_min_element(ack_waiting_list);
                    int removed_index = erase_element(&ack_waiting_list, waiting_ack_no);
                    if (removed_index > ack_waiting_list.size()) {
                        // DELETION FAILED.
                        // perror("COULD NOT DELETE SEQUENCE NUMBER");
                        quit("COULD NOT DELETE SEQUENCE NUMBER");
                    } else {
                        std::cout << "ELEMENT REMOVED FROM --> " << removed_index << std::endl;
                        std::cout << "NOW WAITING ON --> " << waiting_ack_no << std::endl;
                    }
                    sendfromlast = 1;
                } else {
                    // NOT WAITING FOR ANY.
                    isWaitingSet = 0;
                }
                if (total_sent == fsize && ack_waiting_list.size()) {
                    goto RECEIVE;
                }
                // GOTO SEND_PACKET  // NOT SURE
                // goto SEND_PACKET;
                //
            } else {
                // GRAB ANOTHER PACKET
                goto RECEIVE;
                // GOTO RECEIVE BLOCKING CALL
            }
        } else {
            // THINK
            // // GRAB ANOTHER PACKET
        }
    }

    // SEND A BLANK PACKET TO INDICATE THE END OF FILE.
    total_sent = 0;
    waiting_ack_no = 0;
    padding = '-';
    ack_waiting_list.clear();
    std::free(file);
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
