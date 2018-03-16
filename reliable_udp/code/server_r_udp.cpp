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
        // shutdown (listen_socket, SHUT_RDWR);
        // close(listen_socket);
        std::cout << "******HERE******" << '\n';
        std::cout << "count--> " << count++ << '\n';
        std::cout << '\n';
    }

    // -----------------------------------------------------------------------

    return 0;
}

// -----------------------------------------------------------------------
void service_request(int connFD) {
    int sum = 0;
    std::cout << '\n';
    std::cout << "SERVING NOW.." << '\n';
    char receive_buffer[MAXBUFFERSIZE];
    int isError = 0;
    int received_bytes = 0;
    memset(receive_buffer, 0, sizeof receive_buffer);
    received_bytes = recvfrom(connFD, receive_buffer, MAXBUFFERSIZE, 0,
                              (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen);

    if (received_bytes < 0) {
        std::cout << "rc --> " << received_bytes << '\n';
        quit("Error in Readline()");
    } else if (received_bytes > MAXBUFFERSIZE) {
        isError = 1;
    }
    char *break_request[3];
    break_request[0] = strtok(receive_buffer, " ");
    // std::cout << "break_request[0] --> " <<  break_request[0] << '\n';
    if (isError) {
        sendto(connFD, "HTTP/1.0 400 Bad Request", 25, 0, (struct sockaddr *)&clientaddr,
               clientlen);
        sendto(connFD, "Length greater than 255", 24, 0, (struct sockaddr *)&clientaddr, clientlen);
        sendto(connFD, "x", 0, 0, (struct sockaddr *)&clientaddr, clientlen);
        return;
    }
    if (strcmp(break_request[0], "GET") == 0) {
        break_request[1] = strtok(NULL, " ");
        break_request[2] = strtok(NULL, " ");
        Trim(break_request[2]);

        if (strcmp(break_request[2], "HTTP/1.0") != 0 &&
            strcmp(break_request[2], "HTTP/1.1") != 0) {
            std::cout << "AKASH" << '\n';
            sendto(connFD, "HTTP/1.0 400 Bad Request", 24, 0, (struct sockaddr *)&clientaddr,
                   clientlen);
            sendto(connFD, "incorrect http request format", 30, 0, (struct sockaddr *)&clientaddr,
                   clientlen);
            sum += 24 + 30;
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
                std::stringstream sstream;
                std::string tempstr;
                sstream << "HTTP/1.0 200 OK\n\n";
                // std::cout << "sstream --> " << sstream.str() << '\n';
                sendto(connFD, "HTTP/1.0 200 OK\n", 17, 0, (struct sockaddr *)&clientaddr,
                       clientlen);
                sum = sum + 17;
                while (std::getline(file, tempstr)) {
                    // std::cout << "tempstr --> " << tempstr<< '\n';
                    sum = sum + tempstr.length();
                    sendto(connFD, tempstr.c_str(), tempstr.length(), 0,
                           (struct sockaddr *)&clientaddr, clientlen);
                }

            } else {
                std::stringstream ss1;
                ss1 << "HTTP/1.0 404 Not Found\n<HTML>\n<HEAD>\n<TITLE>Server Error" << 404
                    << "</TITLE>\n</HEAD>\n\n";
                ss1 << "<BODY>\n<H1>Server Error: 404</H1>\n";
                ss1 << "<H1>Server Message: File not found </H1>\n ";
                ss1 << "<P>The request could not be completed.</P>\n</BODY>\n</HTML>\n";
                sendto(connFD, ss1.str().c_str(), ss1.str().length(), 0,
                       (struct sockaddr *)&clientaddr, clientlen);
                sum += ss1.str().length();
            }
        }

    } else {
        // std::cout << "here" << '\n';
        sendto(connFD, "HTTP/1.0 400 Bad Request", 25, 0, (struct sockaddr *)&clientaddr,
               clientlen);
        sendto(connFD, "incorrect http request format", 30, 0, (struct sockaddr *)&clientaddr,
               clientlen);
        sendto(connFD, "x", 0, 0, (struct sockaddr *)&clientaddr, clientlen);
        sum += 25 + 30 + 0;
    }
    std::cout << "total sent --> " << sum << '\n';
    sendto(connFD, "x", 0, 0, (struct sockaddr *)&clientaddr, clientlen);
    sendto(connFD, "**over**", 8, 0, (struct sockaddr *)&clientaddr, clientlen);
    // return 0;
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
        // sendto(sockd, buffer, nleft, 0, (struct sockaddr *) &clientaddr, clientlen);
        if ((nwritten =
                 sendto(sockd, buffer, nleft, 0, (struct sockaddr *)&clientaddr, clientlen)) <= 0) {
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
    std::cout << "IN READLINE" << '\n';
    std::cout << "MAXLEN--> " << maxlen << '\n';
    ssize_t n, rc;
    char c, *buffer;

    buffer = (char *)vptr;

    for (n = 1; n < maxlen; n++) {
        std::cout << n << '\n';
        std::cout << "rc --> " << rc << " " << buffer << '\n';
        if ((rc = recvfrom(connFD, buffer, MAXBUFFERSIZE, 0, (struct sockaddr *)&clientaddr,
                           (socklen_t *)&clientlen)) > 0) {
            std::cout << "rc --> " << rc << " " << buffer << '\n';
            // *buffer++ = c;
            /*
             * gethostbyaddr: determine who sent the datagram
             */
            hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                                  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
            if (hostp == NULL) {
                // error("ERROR on gethostbyaddr");
                perror("Error on gethostbyaddr");
                exit(EXIT_FAILURE);
            }

            hostaddrp = inet_ntoa(clientaddr.sin_addr);
            if (hostaddrp == NULL) {
                // error("ERROR on inet_ntoa\n");
                perror("ERROR on inet_ntoa\n");
                exit(EXIT_FAILURE);
            }
            printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);
            std::cout << "server received datagram from " << hostp->h_name << " --> " << hostaddrp
                      << '\n';
            // std::cout << "server received --> " << strlen(buffer)/n <<  '\n';
            // printf("server received %d/%d bytes: %s\n", strlen(buffer), n, buffer);

            // if ( c == '\n' )
            //        break;
        } else if (rc == 0) {
            std::cout << "rc --> " << rc << '\n';
            if (n == 1)
                return 0;
            else
                break;
        } else {
            std::cout << "rc --> " << rc << '\n';
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
