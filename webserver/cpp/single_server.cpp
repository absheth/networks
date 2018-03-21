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
#include <algorithm>
#include <fstream>
#include <sstream>
// #include <thread> // UNCOMMENT FOR THREADED
void perror(const char *str);

// void sigchild_handler(int s);
void service_request(int connFD);
void *get_in_addr(struct sockaddr *sa);
void quit(std::string p_error_message);
int Trim(char *buffer);
int start_server();
ssize_t Writeline(int sockd, const void *vptr, size_t n);
ssize_t readline(int connFD, void *vptr, size_t maxlen);

int parse_request(std::string buffer, std::string &http_req, int *count, int *connection_type,
                  int *client_parameter);
int StrUpper(char *buffer);
void trim(std::string &s);
#define PORT "9158"
#define SUCCESS 0
#define FAILURE -1
#define BACKLOG 5
#define MAXBUFFERSIZE 999999
// #define MAXBUFFERSIZE 99999 // FOR THREADED SERVER
#define PERSISTENT 1
#define NONPERSISTENT 2

std::string working_directory;
int listen_socket;
int main(int argc, char const *argv[]) {
    std::cout << "argc --> " << argc << '\n';
    working_directory = getenv("PWD");
    //--------------------------------------------
    // Variable
    int communicate_socket;
    socklen_t sin_size;
    char clientIP[INET6_ADDRSTRLEN];
    struct sockaddr_storage clientAddr;
    //--------------------------------------------
    if (start_server() != 0) {
        std::cout << "Server did not start." << '\n';
        return EXIT_FAILURE;
    }
    //--------------------------------------------
    int count = 0;
    sin_size = sizeof clientAddr;
    while (1) {
        std::cout << "Now listening.. " << '\n';
        communicate_socket = accept(listen_socket, (struct sockaddr *)&clientAddr, &sin_size);
        if (communicate_socket == FAILURE) {
            perror("Accept error");
            continue;
        }
        inet_ntop(clientAddr.ss_family, get_in_addr((struct sockaddr *)&clientAddr), clientIP,
                  sizeof clientIP);
        std::cout << "Connection from --> " << clientIP << '\n';
        std::cout << "communicate_socket -->" << communicate_socket << '\n';

        service_request(communicate_socket);

        /* For threaded server */
        // std::thread service_thread(service_request, communicate_socket);
        // service_thread.join();
        // std::cout << "count--> " << count++ << '\n';
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "----------------------------Request served: THIS DIVIDES PERSISTENT AND "
                     "NON-PERSISTENT----------------------------"
                  << '\n';

        std::cout << std::endl;
        std::cout << std::endl;
    }

    // -----------------------------------------------------------------------

    return 0;
}

// -----------------------------------------------------------------------

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
// -----------------------------------------------------------------------
void service_request(int connFD) {
    int connection_type = 10;
    char receive_buffer[MAXBUFFERSIZE];
    std::string http_req;
    int req_len = 0;
SERVENEXT:
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "****** SERVING NOW ******" << '\n';
    // std::cout << "connFD --> " << connFD << '\n';

    int count = 0;
    int client_parameter = 0;  // less than 2 means terminal else browser.
    std::string temp_str;
    temp_str = "";
    while (temp_str != "\r\n") {
        /* code */
        memset(receive_buffer, 0, sizeof receive_buffer);
        req_len = readline(connFD, receive_buffer, MAXBUFFERSIZE - 1);
        // Trim(receive_buffer);

        temp_str = receive_buffer;
        trim(temp_str);
        std::cout << "temp_str --> " << temp_str << '\n';
        std::cout << "length--> " << temp_str.length() << '\n';
        // std::cout << "temp str --> " << temp_str<< " -- "  <<temp_str.compare("!!!!!") <<'\n';
        // std::cout << "CONNECTION TYPE --> " << connection_type <<  '\n';
        if (connection_type == PERSISTENT && temp_str.compare("!!!!!") == 0) {
            std::cout << "CLOSING PERSISTENT CONNECTION --> " << connFD << '\n';
            close(connFD);
            return;
        }
        parse_request(temp_str, http_req, &count, &connection_type, &client_parameter);
    }

    std::cout << "CONNECTION TYPE --> "
              << (connection_type == PERSISTENT ? "PERSISTENT" : "NONPERSISTENT") << '\n';
    // exit(EXIT_SUCCESS);
    std::cout << "CLIENT PARAMETER --> " << client_parameter << std::endl;

    char *break_request[3];
    char temp[255];
    strcpy(temp, http_req.c_str());
    break_request[0] = strtok(temp, " ");
    if (strcmp(break_request[0], "GET") == 0) {
        break_request[1] = strtok(NULL, " ");
        break_request[2] = strtok(NULL, " ");
        Trim(break_request[2]);
        if (strcmp(break_request[2], "HTTP/1.0") != 0 &&
            strcmp(break_request[2], "HTTP/1.1") != 0) {
            // send(connFD, "HTTP/1.1 400 Bad Request", 25, 0);

            if (connection_type == NONPERSISTENT) {
                send(connFD, "HTTP/1.1 400 Bad Request", 25, 0);
                // send(connFD, "x", 1,  0);
                // shutdown (connFD, SHUT_RDWR);
                close(connFD);
            } else {
                // std::cout << "****** SENDING END ******" << '\n';
                send(connFD, "HTTP/1.1 400 Bad Requestx!", 27, 0);
                // send(connFD, "x!", 2,  0);
            }
            std::cout << "SENT FAILURE RESPONSE" << '\n';
        } else {
            std::stringstream ss;
            ss << working_directory;
            ss << break_request[1];
            std::cout << "Final path --> " << ss.str() << '\n';
            // std::string path = ss.str().c_str();
            std::ifstream file;
            file.open(ss.str().c_str());
            if (file)  // FILE FOUND
            {
                send(connFD, "HTTP/1.0 200 OK\n\n", 17, 0);

                std::string tempstr;
                while (std::getline(file, tempstr)) {
                    send(connFD, tempstr.c_str(), tempstr.length(), 0);
                }

                if (connection_type == NONPERSISTENT) {
                    close(connFD);
                } else {
                    if (client_parameter < 3) {
                        send(connFD, "x!", 2, 0);
                    }
                }
                std::cout << "SENT SUCCESS RESPONSE" << '\n';
            } else {
                std::cout << "NOT FOUND" << '\n';
                // char buffer[100];
                // sprintf(buffer, "HTTP/1.0 404 Not Found\n<HTML>\n<HEAD>\n<TITLE>Server Error
                // %d</TITLE>\n"
                //         "</HEAD>\n\n", 404);
                // Writeline(connFD, buffer, strlen(buffer));
                //
                // sprintf(buffer, "<BODY>\n<H1>Server Error: %d </H1>\n", 404);
                // Writeline(connFD, buffer, strlen(buffer));
                // sprintf(buffer, "<BODY>\n<H1>Server Message: %s </H1>\n", "File not found");
                // Writeline(connFD, buffer, strlen(buffer));
                // sprintf(buffer, "<P>The request could not be completed.</P>\n"
                //         "</BODY>\n</HTML>\n");
                //
                std::stringstream ss1;
                ss1 << "HTTP/1.0 404 Not Found\n<HTML>\n<HEAD>\n<TITLE>Server Error 404 "
                       "</TITLE>\n</HEAD>\n\n";
                ss1 << "<BODY>\n<H1>Server Error: 404</H1>\n";
                ss1 << "<H1>Server Message: File not found </H1>\n ";
                ss1 << "<P>The request could not be completed.</P>\n</BODY>\n</HTML>\n";
                // sendto(connFD, ss1.str().c_str(), ss1.str().length(), 0, (struct sockaddr *)
                // &clientaddr, clientlen);
                Writeline(connFD, ss1.str().c_str(), ss1.str().length());
                if (connection_type == NONPERSISTENT) {
                    close(connFD);
                } else {
                    if (client_parameter < 3) {
                        send(connFD, "x!", 2, 0);
                    }
                }

                std::cout << "SENT FAILURE RESPONSE" << '\n';
            }
        }
    }
    if (connection_type == PERSISTENT) {
        if (client_parameter > 2) {
            close(connFD);
            return;
        }
        std::cout << "THE CONNECTION IS PERSISTENT." << '\n';

        goto SERVENEXT;
    }
}
// -----------------------------------------------------------------------
/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *buffer;

    buffer = (char *)vptr;
    nleft = n;

    while (nleft > 0) {
        if ((nwritten = write(sockd, buffer, nleft)) <= 0) {
            if (errno == EINTR)
                nwritten = 0;
            else
                quit("Error in Writeline()");
        }
        nleft -= nwritten;
        buffer += nwritten;
    }

    return n;
}
// -----------------------------------------------------------------------
/*  Read a line from a socket  */

ssize_t readline(int connFD, void *vptr, size_t maxlen) {
    std::cout << " -- READING BUFFER --" << '\n';
    ssize_t n, rc;
    char c, *buffer;

    buffer = (char *)vptr;
    memset((char *)buffer, 0, sizeof buffer);
    for (n = 1; n < maxlen; n++) {
        if ((rc = read(connFD, &c, 1)) == 1) {
            *buffer++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1)
                return 0;
            else
                break;
        } else {
            if (errno == EINTR)
                continue;
            quit("Error in Readline()");
        }
    }

    *buffer = 0;
    return n;
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
int parse_request(std::string buffer, std::string &http_req, int *count, int *connection_type,
                  int *client_parameter) {
    if (*count == 0) {
        http_req = buffer;
        (*count)++;
        return 0;
    }
    (*client_parameter)++;
    char str_array[255];
    strcpy(str_array, buffer.c_str());
    char *break_request = strtok(str_array, ":");
    StrUpper(break_request);
    std::string y(break_request);
    trim(y);

    // Checking for connection type.
    if (!strcmp(y.c_str(), "CONNECTION")) {
        break_request = strtok(NULL, " ");
        StrUpper(break_request);

        std::string z(break_request);
        z.erase(std::remove(z.begin(), z.end(), '\n'), z.end());
        // std::cout << "z.compare(CLOSE) --> "<< (z.compare("CLOSE")) << '\n';
        if (z.compare("CLOSE") == 0) {
            *connection_type = NONPERSISTENT;
            // std::cout << "connection set --> NONPERSISTENT " << *connection_type << '\n';
        } else {
            *connection_type = PERSISTENT;
            // std::cout << "connection set --> PERSISTENT" << *connection_type << '\n';
        }
    }

    return 0;
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
    int enable = 1;
    struct addrinfo hints, *server_info, *pointer;
    int getAddrRet;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // USE MY IP
    // std::string port(PORT);
    if ((getAddrRet = getaddrinfo(NULL, PORT, &hints, &server_info)) != SUCCESS) {
        std::cerr << "getaddrinfo error -->" << gai_strerror(getAddrRet) << '\n';
        return EXIT_FAILURE;
    }

    // loop through all the results and bind to the first we can
    for (pointer = server_info; pointer != NULL; pointer = server_info->ai_next) {
        if ((listen_socket = socket(pointer->ai_family, pointer->ai_socktype,
                                    pointer->ai_protocol)) == FAILURE) {
            std::perror("Socket Error ");
            continue;  // because we want to keep checking for other IP Addresses.
        }
        //-- CHECK
        if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
            std::perror("setsockopt error");
            return FAILURE;
        }
        //-- CHECK

        if (bind(listen_socket, pointer->ai_addr, pointer->ai_addrlen) == -1) {
            close(listen_socket);  // close the listening socket because the binding failed.
            std::perror("server bind error");
            continue;
        }
        break;
    }

    if (pointer == NULL) {
        std::perror("Server failed to bind");
        return FAILURE;
    }

    freeaddrinfo(server_info);  // USE COMPLETE

    if (listen(listen_socket, BACKLOG) == FAILURE) {
        std::perror("listen error");
        return FAILURE;
    }

    std::cout << "Server started. Waiting for the connection." << '\n';
    return SUCCESS;
}
