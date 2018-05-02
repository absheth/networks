#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <climits>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#define BUFFER_SIZE 256
#define BIG_NUM 999999999
typedef struct {
    char Dst[50];
    char nxtHop[50];
    int cost;
    u_short ttl;
} route_entry;
typedef struct {
    route_entry table[4];
} Route;
int main(int argc, char *argv[]) {
    struct sockaddr_in server;
    struct addrinfo hints;
    struct addrinfo *result, *result_list;
    std::string request_string;
    ssize_t char_read;
    char receive_buffer[BUFFER_SIZE];
    int client_fd, port;
    // std::map<std::string, Route> send_map;
    route_entry route_table_A[5];
    route_entry route_table_B[5];
    route_entry route_table_C[5];
    route_entry route_table_D[5];
    route_entry route_table_E[5];
    // Dummy route
    {
        // A_ROUTE
        memset(route_table_A, 0, (sizeof(route_table_A)));
        strcpy(&route_table_A[0].Dst[0], "A");
        strcpy(&route_table_A[0].nxtHop[0], "A");
        route_table_A[0].cost = 0;
        route_table_A[0].ttl = 90;
        strcpy(&route_table_A[1].Dst[0], "B");
        strcpy(&route_table_A[1].nxtHop[0], "B");
        route_table_A[1].cost = 1;
        route_table_A[1].ttl = 90;
        strcpy(&route_table_A[2].Dst[0], "C");
        strcpy(&route_table_A[2].nxtHop[0], "C");
        route_table_A[2].cost = 1;
        route_table_A[2].ttl = 90;
        strcpy(&route_table_A[3].Dst[0], "D");
        strcpy(&route_table_A[3].nxtHop[0], "D");
        route_table_A[3].cost = BIG_NUM;
        route_table_A[3].ttl = 90;
        strcpy(&route_table_A[4].Dst[0], "E");
        strcpy(&route_table_A[4].nxtHop[0], "E");
        route_table_A[4].cost = BIG_NUM;
        route_table_A[4].ttl = 90;

        // B_ROUTE
        memset(route_table_B, 0, (sizeof(route_table_B)));
        strcpy(&route_table_B[0].Dst[0], "A");
        strcpy(&route_table_B[0].nxtHop[0], "A");
        route_table_B[0].cost = 1;
        route_table_B[0].ttl = 90;
        strcpy(&route_table_B[1].Dst[0], "B");
        strcpy(&route_table_B[1].nxtHop[0], "B");
        route_table_B[1].cost = 0;
        route_table_B[1].ttl = 90;
        strcpy(&route_table_B[2].Dst[0], "C");
        strcpy(&route_table_B[2].nxtHop[0], "C");
        route_table_B[2].cost = BIG_NUM;
        route_table_B[2].ttl = 90;
        strcpy(&route_table_B[3].Dst[0], "D");
        strcpy(&route_table_B[3].nxtHop[0], "D");
        route_table_B[3].cost = 1;
        route_table_B[3].ttl = 90;
        strcpy(&route_table_B[4].Dst[0], "E");
        strcpy(&route_table_B[4].nxtHop[0], "E");
        route_table_B[4].cost = BIG_NUM;
        route_table_B[4].ttl = 90;


        // C_ROUTE
        memset(route_table_C, 0, (sizeof(route_table_C)));
        strcpy(&route_table_C[0].Dst[0], "A");
        strcpy(&route_table_C[0].nxtHop[0], "A");
        route_table_C[0].cost = 1;
        route_table_C[0].ttl = 90;
        strcpy(&route_table_C[1].Dst[0], "B");
        strcpy(&route_table_C[1].nxtHop[0], "B");
        route_table_C[1].cost = BIG_NUM;
        route_table_C[1].ttl = 90;
        strcpy(&route_table_C[2].Dst[0], "C");
        strcpy(&route_table_C[2].nxtHop[0], "C");
        route_table_C[2].cost = 0;
        route_table_C[2].ttl = 90;
        strcpy(&route_table_C[3].Dst[0], "D");
        strcpy(&route_table_C[3].nxtHop[0], "D");
        route_table_C[3].cost = 1;
        route_table_C[3].ttl = 90;
        strcpy(&route_table_C[4].Dst[0], "E");
        strcpy(&route_table_C[4].nxtHop[0], "E");
        route_table_C[4].cost = BIG_NUM;
        route_table_C[4].ttl = 90;


        // D_ROUTE
        memset(route_table_D, 0, (sizeof(route_table_D)));
        strcpy(&route_table_D[0].Dst[0], "A");
        strcpy(&route_table_D[0].nxtHop[0], "A");
        route_table_D[0].cost = BIG_NUM;
        route_table_D[0].ttl = 90;
        strcpy(&route_table_D[1].Dst[0], "B");
        strcpy(&route_table_D[1].nxtHop[0], "B");
        route_table_D[1].cost = 0;
        route_table_D[1].ttl = 90;
        strcpy(&route_table_D[2].Dst[0], "C");
        strcpy(&route_table_D[2].nxtHop[0], "C");
        route_table_D[2].cost = 1;
        route_table_D[2].ttl = 90;
        strcpy(&route_table_D[3].Dst[0], "D");
        strcpy(&route_table_D[3].nxtHop[0], "D");
        route_table_D[3].cost = 0;
        route_table_D[3].ttl = 90;
        strcpy(&route_table_D[4].Dst[0], "E");
        strcpy(&route_table_D[4].nxtHop[0], "E");
        route_table_D[4].cost = BIG_NUM;
        route_table_D[4].ttl = 90;

        // E_ROUTE
        memset(route_table_E, 0, (sizeof(route_table_E)));
        strcpy(&route_table_E[0].Dst[0], "A");
        strcpy(&route_table_E[0].nxtHop[0], "A");
        route_table_E[0].cost = BIG_NUM;
        route_table_E[0].ttl = 90;
        strcpy(&route_table_E[1].Dst[0], "B");
        strcpy(&route_table_E[1].nxtHop[0], "B");
        route_table_E[1].cost = BIG_NUM;
        route_table_E[1].ttl = 90;
        strcpy(&route_table_E[2].Dst[0], "C");
        strcpy(&route_table_E[2].nxtHop[0], "C");
        route_table_E[2].cost = BIG_NUM;
        route_table_E[2].ttl = 90;
        strcpy(&route_table_E[3].Dst[0], "D");
        strcpy(&route_table_E[3].nxtHop[0], "D");
        route_table_E[3].cost = 1;
        route_table_E[3].ttl = 90;
        strcpy(&route_table_E[4].Dst[0], "E");
        strcpy(&route_table_E[4].nxtHop[0], "E");
        route_table_E[4].cost = 0;
        route_table_E[4].ttl = 90;
    }
    if (argc != 4) {
        std::cout << "Correct usage is: ./" << argv[0]
                  << " server_host server_port node_number(start: A-1, B-2.. )" << std::endl;
        return -1;
    }

    port = std::atoi(argv[2]);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    if (getaddrinfo(argv[1], argv[2], &hints, &result) != 0) {
        std::perror("Getaddrinfo error");
    }

    for (result_list = result; result_list != NULL; result_list = result_list->ai_next) {
        client_fd = socket(result_list->ai_family, result_list->ai_socktype, 0);
        if (client_fd == -1) {
            continue;
        }

        if (connect(client_fd, result_list->ai_addr, result_list->ai_addrlen) != -1) {
            break;
        }

        close(client_fd);
    }

    if (result_list == NULL) {
        std::perror("Error in connection");
    }
    freeaddrinfo(result);

    int len;

    switch (atoi(argv[3])) {
        case 1:
            std::cout << "Size of route_table --> " << sizeof(route_table_A) << std::endl;
            len = sendto(client_fd, &route_table_A, sizeof(route_table_A), 0, result_list->ai_addr,
                         result_list->ai_addrlen);
            break;
        case 2:
            std::cout << "Size of route_table --> " << sizeof(route_table_B) << std::endl;
            len = sendto(client_fd, &route_table_B, sizeof(route_table_B), 0, result_list->ai_addr,
                         result_list->ai_addrlen);
            break;
        case 3:
            std::cout << "Size of route_table --> " << sizeof(route_table_C) << std::endl;
            len = sendto(client_fd, &route_table_C, sizeof(route_table_C), 0, result_list->ai_addr,
                         result_list->ai_addrlen);
            break;
        case 4:
            std::cout << "Size of route_table --> " << sizeof(route_table_D) << std::endl;
            len = sendto(client_fd, &route_table_D, sizeof(route_table_D), 0, result_list->ai_addr,
                         result_list->ai_addrlen);
            break;

        case 5:
            std::cout << "Size of route_table --> " << sizeof(route_table_E) << std::endl;
            len = sendto(client_fd, &route_table_E, sizeof(route_table_E), 0, result_list->ai_addr,
                         result_list->ai_addrlen);
            break;
        default:
            std::cout << "Writing default: B" << std::endl;
            len = sendto(client_fd, &route_table_B, sizeof(route_table_B), 0, result_list->ai_addr,
                         result_list->ai_addrlen);
    }
    // std::cout << "Size of route_table --> " << sizeof(route_table_B) << std::endl;
    // len = write(client_fd, &route_table_A, (sizeof(route_table_A)));
    // len = write(client_fd, &route_table_B, (sizeof(route_table_B)));
    // len = write(client_fd, &route_table_C, (sizeof(route_table_C)));
    // len = write(client_fd, &route_table_D, (sizeof(route_table_D)));

    std::cout << "Bytes written: " << len << std::endl;

    if (len != request_string.length()) {
        std::perror("Write failure");
    }

    close(client_fd);
}
