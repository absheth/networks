#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <climits>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <thread>

// CONSTANTS
#define BIG_NUM 999999999

// GLOBAL VARIABLES
static std::string port_no;
static int period;
bool init_flag = false;
bool triggered_update = false;
bool expired = false;
int ttl;
bool poison = false;
// -----------------------------------------------------------------------
// Routing table entry
typedef struct {
    char Dst[50];
    char nxtHop[50];
    int cost;
    short ttl;
} route_entry;

// Update message structure
typedef struct update_message {
    char Dst[50];
    int cost;
} update_message;

// Route Table and it's mutex
std::map<std::string, route_entry> route_table;
std::mutex route_table_mutex;

// Current node
std::string current_node;

// Graph Adj list
std::map<std::string, std::map<std::string, int> > adjList;

// -----------------------------------------------------------------------
// PRINTING
void printRoutingTable() {
    // Take mutex lock for route table
    std::lock_guard<std::mutex> guard(route_table_mutex);

    std::cout << std::endl;
    std::cout << "*** Route table ***" << std::endl;
    for (auto route : route_table) {
        std::cout << "Dst: " << route.second.Dst << " Nxt: " << route.second.nxtHop
                  << " cost: " << route.second.cost << " ttl: " << route.second.ttl << std::endl;
    }
    std::cout << std::endl;
}

// Print graph
void printGraph() {
    std::cout << std::endl;
    std::cout << "*******" << std::endl;
    std::cout << "Print graph: " << std::endl;

    for (auto it : adjList) {
        for (auto iter : it.second) {
            std::cout << it.first << " --> " << iter.first << " | cost: " << iter.second
                      << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << "*******" << std::endl;
}

// -----------------------------------------------------------------------

// IP to hostname resolution
std::string reverseLookup(char *client) {
    struct sockaddr_in sa;
    char node[NI_MAXHOST];
    std::string test;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;

    inet_pton(AF_INET, client, &sa.sin_addr);

    int res =
        getnameinfo((struct sockaddr *)&sa, sizeof(sa), node, sizeof(node), NULL, 0, NI_NAMEREQD);

    test = node;
    test = test.substr(0, test.find("."));

    return test;
}

// -----------------------------------------------------------------------
void sendWrapper(char *host, update_message *data) {
    struct addrinfo hints;
    struct addrinfo *result, *result_list;
    int client_fd;
    char *port;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    if (getaddrinfo(host, port_no.c_str(), &hints, &result) != 0) {
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
    len = sendto(client_fd, data, (sizeof(update_message) * (route_table.size())), 0,
                 result_list->ai_addr, result_list->ai_addrlen);

    close(client_fd);
}

// -----------------------------------------------------------------------
void createUpdate(std::string forDst, update_message *update) {
    int x = 0;
    for (auto iter : route_table) {
        strcpy(update[x].Dst, iter.second.Dst);

        if (poison) {
            if (iter.second.nxtHop == forDst && iter.second.cost != 1) {
                update[x].cost = BIG_NUM;
            } else {
                update[x].cost = iter.second.cost;
            }
        } else {
            update[x].cost = iter.second.cost;
        }
        ++x;
    }
}

// -----------------------------------------------------------------------
void sendAdvertisement() {
    // Acquire mutex lock on route table
    route_table_mutex.lock();

    // Prepare array to send from reading the routing table
    int count = route_table.size();
    update_message update[count];

    for (auto &iter : route_table) {
        // Handle first iteration
        if (!init_flag && !triggered_update &&
            strcmp(current_node.c_str(), iter.first.c_str()) != 0) {
            iter.second.ttl = iter.second.ttl - period;
            if (iter.second.ttl <= 0) {
                iter.second.cost = BIG_NUM;
                strcpy(iter.second.nxtHop, "null");
                expired = true;
            }
        }
    }

    for (auto &iter : route_table) {
        // Decide to send
        if (strcmp(current_node.c_str(), iter.first.c_str()) != 0 && iter.second.cost == 1) {
            // Call create update for iter.first as dst
            createUpdate(iter.first, update);
            sendWrapper(iter.second.Dst, update);
        }
    }

    // Release mutex lock on route table
    route_table_mutex.unlock();
}

// -----------------------------------------------------------------------
// Worker thread
void sendThread(int period) {
    while (1) {
        sleep(period);
        sendAdvertisement();
    }
}

// -----------------------------------------------------------------------
// Initialize
void init(std::string ConfigFile) {
    // Reading the conf file and filling the inital graph according to Bellman-Ford's initial
    // condition
    std::string in, in_2, node, node_insert, pathExists;
    std::ifstream conf(ConfigFile);
    std::ifstream conf_2(ConfigFile);
    std::map<std::string, int> tmpList;

    // Insert the first entry in the graph for itself
    while (getline(conf, in)) {
        node = in.substr(0, in.find(" "));
        pathExists = in.substr(in.find(" ") + 1);

        if (pathExists == "true") {
            tmpList.insert(std::make_pair(node, 1));
        } else {
            tmpList.insert(std::make_pair(node, BIG_NUM));
        }
    }
    adjList.insert(std::make_pair(current_node, tmpList));

    // Iterate over the file to get neighbours and initialize all other values to infinity in the
    // graph
    conf.clear();
    conf.seekg(0, std::ios::beg);
    tmpList.clear();
    while (getline(conf, in_2)) {
        while (getline(conf_2, in)) {
            if (in_2 == in) {
                node = in.substr(0, in.find(" "));
                node_insert = node;
                tmpList.insert(std::make_pair(current_node, BIG_NUM));
            } else {
                node = in.substr(0, in.find(" "));
                tmpList.insert(std::make_pair(node, BIG_NUM));
            }
        }
        adjList.insert(std::make_pair(node_insert, tmpList));
        conf_2.clear();
        conf_2.seekg(0, std::ios::beg);
        tmpList.clear();
    }
    conf.close();
    conf_2.close();

    route_entry temp_entry;
    strcpy(temp_entry.Dst, current_node.c_str());
    strcpy(temp_entry.nxtHop, current_node.c_str());
    temp_entry.cost = 0;
    temp_entry.ttl = ttl;
    route_table.insert(std::make_pair(current_node, temp_entry));
    for (auto iter : adjList[current_node]) {
        strcpy(temp_entry.Dst, iter.first.c_str());
        if (iter.second != BIG_NUM) {
            strcpy(temp_entry.nxtHop, iter.first.c_str());
        } else {
            strcpy(temp_entry.nxtHop, "null");
        }
        temp_entry.cost = iter.second;
        temp_entry.ttl = ttl;
        route_table.insert(std::make_pair(iter.first, temp_entry));
    }

    printRoutingTable();
    printGraph();
    // Send_Adverstisement
    init_flag = true;
    sendAdvertisement();
    init_flag = false;
}

// -----------------------------------------------------------------------
void update_routes(std::string route_from, update_message *received_table,
                   struct sockaddr_in &clientAddr, int size_of_table) {
    int route_update = 0;
    // Route Handlings
    // Update adjacent node cost to 1.
    adjList[current_node][route_from] = 1;
    // ----------------------------------------

    // Acquire mutex lock for route table
    route_table_mutex.lock();

    route_table.at(route_from).ttl = ttl;
    route_table.at(route_from).cost = 1;
    strcpy(route_table.at(route_from).nxtHop, route_from.c_str());

    for (int i = 0; i < size_of_table; i++) {
        if (received_table[i].cost == BIG_NUM) {
            adjList[route_from][received_table[i].Dst] = received_table[i].cost;
            if (route_table[received_table[i].Dst].cost != 1) {
                strcpy(route_table[received_table[i].Dst].nxtHop, "null");
                route_table[received_table[i].Dst].cost = BIG_NUM;
                route_table[received_table[i].Dst].ttl = -1;
            }
        }
        if (received_table[i].cost < adjList[route_from][received_table[i].Dst]) {
            adjList[route_from][received_table[i].Dst] = received_table[i].cost;
        }
    }
    // ----------------------------------------
    // Bellman Ford

    // Updating TTL for unreachable nodes.

    for (int i = 0; i < size_of_table; i++) {
        if (strcmp(route_table[received_table[i].Dst].nxtHop, route_from.c_str()) == 0 &&
            received_table[i].cost != BIG_NUM) {
            route_table[received_table[i].Dst].ttl = ttl;
        }
    }

    for (auto iter : adjList[current_node]) {
        std::pair<std::string, int> min_pair =
            std::make_pair(current_node, route_table[iter.first].cost);
        int local_cost_sum = 0;

        local_cost_sum = std::min(min_pair.second,
                                  route_table[route_from].cost + adjList[route_from][iter.first]);

        if (local_cost_sum < min_pair.second) {
            min_pair = std::make_pair(route_from, local_cost_sum);
            strcpy(route_table[iter.first].nxtHop, min_pair.first.c_str());
            route_table[iter.first].cost = min_pair.second;
            route_table[iter.first].ttl = ttl;
            route_update = 1;  // Change handling.
        }
    }
    // Releasing route table mutex lock
    route_table_mutex.unlock();
    if (route_update) {
        std::cout << std::endl;

        std::cout << "----------------------------" << std::endl;
        std::cout << "After updating route table: " << std::endl;

        printRoutingTable();
        std::cout << "----------------------------" << std::endl;
        triggered_update = true;
        sendAdvertisement();
        triggered_update = false;
    }
}

// -----------------------------------------------------------------------

int main(int argc, char *argv[]) {
    // Declarations
    std::cout << std::endl;
    std::string Conf;
    int bytes_received = 0;
    struct sockaddr_in clientAddr;
    socklen_t clientlen = 0;
    // ----------------------------------------
    if (argc != 6) {
        std::cout << "Usage: " << argv[0] << " <.conf> <port_num> <ttl> <period> <poison>"
                  << std::endl;
        return -1;
    }

    // ----------------------------------------
    // Get current hostname
    int length = 100;
    char hostname[100];
    gethostname(hostname, length);
    std::string tmp(hostname);
    current_node = tmp.substr(0, tmp.find("."));
    // ----------------------------------------

    Conf = argv[1];
    port_no = argv[2];
    ttl = std::atoi(argv[3]);
    period = std::atoi(argv[4]);
    poison = (std::atoi(argv[5]) == 0 ? false : true);

    init(Conf);
    std::thread send_worker(sendThread, period);
    send_worker.detach();

    update_message received_table[route_table.size()];

    // ----------------------------------------
    // Set up UDP
    struct sockaddr_in server, client;
    socklen_t len;
    int server_fd;

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd == -1) {
        std::perror("Socket creation failed");
        return -1;
    }

    // Clearing the socket structure
    memset(&server, 0, sizeof(struct sockaddr_in));

    // Define socket parameters
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(std::atoi(port_no.c_str()));

    // Binding the socket
    if (bind(server_fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
        std::perror("Socket bind failed");
        close(server_fd);
        return -1;
    }
    // ----------------------------------------

    // Main computes routes on receiving update on socket
    std::string lookupHost;
    while (true) {
        // ----------------------------------------
        if (expired) {
            std::cout << std::endl;
            std::cout << "----------------------------" << std::endl;
            std::cout << "Route expired" << std::endl;
            printRoutingTable();
            std::cout << "----------------------------" << std::endl;

            expired = false;
        }
        // Re-Calculate network view graph on receiving update
        std::memset(received_table, 0, sizeof(received_table));
        clientlen = sizeof(clientAddr);
        bytes_received = recvfrom(server_fd, received_table, (sizeof(received_table)), 0,
                                  (struct sockaddr *)(&clientAddr), (socklen_t *)&clientlen);

        // ----------------------------------------
        char *clientIP;
        int port;
        clientIP = inet_ntoa(clientAddr.sin_addr);
        port = clientAddr.sin_port;
        lookupHost = reverseLookup(clientIP);
        // ----------------------------------------
        // Update Route Weights
        update_routes(lookupHost, received_table, clientAddr,
                      sizeof(received_table) / sizeof(update_message));
        // ----------------------------------------
    }

    return 0;
}
