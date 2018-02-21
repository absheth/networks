// Header file for input output functions
#include <iostream>
// #include <stdio.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sstream>
#include <fstream>

void perror ( const char * str );
void getIPAddr();
// void sigchild_handler(int s);
void service_request(int connFD);
void *get_in_addr(struct sockaddr *sa);
void quit(std::string p_error_message);
int Trim(char * buffer);
int start_server();
ssize_t Writeline(int sockd, const void *vptr, size_t n);
ssize_t readline(int connFD, void *vptr, size_t maxlen);
int parse_request(std::string buffer, std::string &http_req, int* count, int* connection_type);

int StrUpper(char * buffer);
void trim(std::string& s);
#define PORT "9158"  // the port users will be connecting to
#define SUCCESS 0
#define FAILURE -1
#define BACKLOG 5
#define MAXBUFFERSIZE 999999
#define PERSISTENT 1
#define NONPERSISTENT 2

std::string working_directory;
int listen_socket;
int main(int argc, char const *argv[]) {
        std::cout << "argc --> " << argc << '\n';
        working_directory = getenv("PWD");
        std::cout << "working_directory --> " << working_directory <<'\n';
        getIPAddr();
        //--------------------------------------------
        // Variable

        int communicate_socket;
        socklen_t sin_size;
        char clientIP[INET6_ADDRSTRLEN];
        struct sockaddr_storage clientAddr; // This stores the client address information.
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
                // int read;
                // read = read(connFD, &c, 1);
                // std::cout << "read --> " << read << " char --> " << c <<  '\n';
                // if(read == 1) {
                //         if ((c - '0') == 1) {
                //                 connection_type = PERSISTENT;
                //         } else {
                //                 connection type = NONPERSISTENT;
                //         }
                // } else {
                //         std::perror("incorrect connection type");
                // }



                inet_ntop(clientAddr.ss_family, get_in_addr((struct sockaddr *)&clientAddr), clientIP, sizeof clientIP);
                std::cout << "Connection from --> " << clientIP <<'\n';
                std::cout << "communicate_socket -->" << communicate_socket << '\n';
                service_request(communicate_socket);

                std::cout << "******HERE******" << '\n';
                std::cout << "count--> " << count++ << '\n';



        }

        // -----------------------------------------------------------------------

        return 0;
}

// -----------------------------------------------------------------------
// void sigchild_handler(int s)
// {
//         while(waitpid(-1, NULL, WNOHANG) > 0) ;
// }
// -----------------------------------------------------------------------
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
        if (sa->sa_family == AF_INET) {
                return &(((struct sockaddr_in*)sa)->sin_addr);
        }

        return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
// -----------------------------------------------------------------------
void service_request(int connFD) {
        int connection_type = 10;
        SERVENEXT:
        std::cout << "****** SERVING NOW ******" << '\n';
        std::cout << "connFD --> " << connFD << '\n';
        // int duration = 5;
        char receive_buffer[MAXBUFFERSIZE];
        // char send_buffer[MAXBUFFERSIZE];
        // char http_req[MAXBUFFERSIZE];
        std::string http_req;
        int rval; //, file_desc, nRead;
        int req_len = 0;
        // char filepath[9999];

        // fd_set fds; // Array of bits // One bit for one file descriptor
        // struct timeval timeout;

        /* Set timeout to 10 seconds */
        // timeout.tv_sec = duration; // Seconds
        // timeout.tv_usec = 0; // Microseconds
        /* Reset file descriptor set */
        // FD_ZERO(&fds); // Clear all the bits
        // FD_SET(connFD, &fds); // Set bit for connFD

        /* Wait until the time out is ready */
        // rval = select(connFD+1, &fds, NULL, NULL, &timeout);

        int count = 0;
        do {
                /* code */

                // if (rval < 0) {
                //         // Error calling select()
                //         quit("Error while calling select()");
                //
                // } else if (rval == 0) {
                //         std::cerr << "TIMEOUT" << '\n';
                //         return -1;
                // } else {

                std::string temp_str;
                req_len = readline(connFD, receive_buffer, MAXBUFFERSIZE - 1);
                //Trim(receive_buffer);

                temp_str = receive_buffer;
                trim(temp_str);
                std::cout << "temp str --> " << temp_str<< " -- "  <<temp_str.compare("!!!!!") <<'\n';
                std::cout << "CONNECTION TYPE --> " << connection_type <<  '\n';
                if (connection_type == PERSISTENT && temp_str.compare("!!!!!") == 0) {
                        std::cout << "CLOSING connFD" << '\n';
                        close(connFD);
                        return;
                }

                parse_request(temp_str, http_req, &count, &connection_type);

                // }
        } while(connection_type == 10);
        std::cout << "receive buffer --> " << receive_buffer << '\n';
        std::cout << "receive buffer size --> " << sizeof receive_buffer << '\n';
        std::cout << "req_len --> " << req_len <<  '\n';
        std::cout << "CONNECTION TYPE --> " << connection_type << '\n';
        std::cout << "CONNECTION TYPE --> " << (connection_type==PERSISTENT ? "PERSISTENT" : "NONPERSISTENT") <<'\n';

        char *break_request[3];
        char temp[255];
        strcpy(temp, http_req.c_str());
        std::cout << "http--> " << http_req << '\n';
        std::cout << "temp--> " << temp;
        break_request[0] = strtok(temp, " ");

        std::cout << "break_request[0] --> " <<  break_request[0] << '\n';
        if (strcmp(break_request[0], "GET") == 0) {
                break_request[1] = strtok(NULL, " ");
                break_request[2] = strtok(NULL, " ");
                Trim(break_request[2]);
                if (strcmp(break_request[2], "HTTP/1.0") != 0 && strcmp(break_request[2], "HTTP/1.1") != 0) {
                        write(connFD, "HTTP/1.0 400 Bad Request", 25);
                } else {
                        std::stringstream ss;
                        ss << working_directory;
                        ss << break_request[1];
                        std::cout << "Final path --> " << ss.str() << '\n';
                        // std::string path = ss.str().c_str();
                        std::ifstream file;
                        file.open(ss.str().c_str());
                        if (file)    //FILE FOUND
                        {
                                send(connFD, "HTTP/1.0 200 OK\n\n", 17, 0);

                                std::string tempstr;
                                while (std::getline(file, tempstr)) {
                                        // output the line
                                        // std::cout << tempstr << std::endl;
                                        // std::cout << "sending --> " << tempstr << " length --> " << tempstr.length() << '\n';
                                        send(connFD, tempstr.c_str(), tempstr.length(),  0);
                                        // now we loop back and get the next line in 'str'
                                }

                                if (connection_type == NONPERSISTENT) {
                                        // send(connFD, "x", 1,  0);
                                        //shutdown (connFD, SHUT_RDWR);
                                        close(connFD);
                                } else {
                                        send(connFD, "x!", 2,  0);
                                }
                                std::cout << "SENT SUCCESS RESPONSE" << '\n';
                        }
                        else  {
                                std::cout << "NOT FOUND" << '\n';
                                char buffer[100];
                                sprintf(buffer, "HTTP/1.0 404 Not Found\n<HTML>\n<HEAD>\n<TITLE>Server Error %d</TITLE>\n"
                                        "</HEAD>\n\n", 404);
                                Writeline(connFD, buffer, strlen(buffer));

                                sprintf(buffer, "<BODY>\n<H1>Server Error: %d </H1>\n", 404);
                                Writeline(connFD, buffer, strlen(buffer));
                                sprintf(buffer, "<BODY>\n<H1>Server Message: %s </H1>\n", "File not found");
                                Writeline(connFD, buffer, strlen(buffer));
                                sprintf(buffer, "<P>The request could not be completed.</P>\n"
                                        "</BODY>\n</HTML>\n");
                                Writeline(connFD, buffer, strlen(buffer));

                                // return 0;
                        }
                }

        } else {
                std::cout << "NOT GET" << '\n';
        }
        if (connection_type == PERSISTENT) {
                std::cout << "THE CONNECTION IS PERSISTENT." << '\n';
                goto SERVENEXT;
        }

        // return 0;
}
// -----------------------------------------------------------------------

void getIPAddr() {
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1) {
                std::perror("getifaddrs");
                exit(EXIT_FAILURE);
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
                family = ifa->ifa_addr->sa_family;

                if (family == AF_INET) {
                        s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                        host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                        if (s != 0) {

                                std::cout << "getnameinfo() failed: " << gai_strerror(s) <<'\n';
                                exit(EXIT_FAILURE);
                        }
                        std::cout << "<Interface>:> " << ifa->ifa_name << "<Address>" << host << '\n';
                }
        }
}


// -----------------------------------------------------------------------
/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n) {
        size_t nleft;
        ssize_t nwritten;
        const char *buffer;

        buffer = (char*)vptr;
        nleft  = n;

        while ( nleft > 0 ) {
                if ( (nwritten = write(sockd, buffer, nleft)) <= 0 ) {
                        if ( errno == EINTR )
                                nwritten = 0;
                        else
                                quit("Error in Writeline()");
                }
                nleft  -= nwritten;
                buffer += nwritten;
        }

        return n;
}
// -----------------------------------------------------------------------
/*  Read a line from a socket  */

ssize_t readline(int connFD, void *vptr, size_t maxlen) {
        std::cout << "IN READLINE" << '\n';
        std::cout << "MAXLEN--> " << maxlen <<'\n';
        ssize_t n, rc;
        char c, *buffer;

        buffer = (char*)vptr;
        memset((char*)buffer, 0, sizeof buffer);
        for ( n = 1; n < maxlen; n++ ) {

                if ( (rc = read(connFD, &c, 1)) == 1 ) {

                        *buffer++ = c;
                        if ( c == '\n' )
                                break;
                }
                else if ( rc == 0 ) {

                        if ( n == 1 )
                                return 0;
                        else
                                break;
                }
                else {

                        if ( errno == EINTR )
                                continue;
                        quit("Error in Readline()");
                }
        }

        *buffer = 0;
        return n;
}
// -----------------------------------------------------------------------

int Trim(char * buffer) {
        int n = strlen(buffer) - 1;

        while ( !isalnum(buffer[n]) && n >= 0 )
                buffer[n--] = '\0';

        return 0;
}

// -----------------------------------------------------------------------
void quit(std::string p_error_message) {

        std::cerr << "p_error_message --> " << p_error_message << '\n';
        exit(EXIT_FAILURE);
}
// -----------------------------------------------------------------------
int parse_request(std::string buffer, std::string &http_req, int* count, int* connection_type) {

        if (*count == 0) {
                // http_req = buffer;

                http_req = buffer;
                (*count)++;

                // std::cout << "IN IF --> COUNT --> " << *count << '\n';
                // std::cout << "IN IF --> HTTP --> " << http_req << '\n';
                // std::cout << "IN IF --> BUFFER --> " << buffer << '\n';
                return 0;
        }
        // std::cout << "OUT IF --> COUNT --> " << *count << '\n';
        // std::cout << "OUT IF --> HTTP --> " << http_req << '\n';
        // std::cout << "OUT IF --> BUFFER --> " << buffer << '\n';




        // std::string temp_str = "   Connection    :   Keep-Alive    ";
        // std::cout << "temp_str --> " << temp_str << '\n';
        // std::string myText("some-text-to-tokenize");
        // std::istringstream iss(temp_str);
        // std::string token;
        // while (std::getline(iss, token, ':'))
        // {
        //         trim(token);
        //         std::cout << token << std::endl;
        // }
        char str_array[255];
        strcpy(str_array, buffer.c_str());
        char * break_request = strtok(str_array, ":");
        StrUpper(break_request);
        std::string y(break_request);
        trim(y);
        // std::cout << "y --> " << y <<'\n';

        //Checking for connection type.
        if ( !strcmp(y.c_str(), "CONNECTION") ) {
                break_request = strtok(NULL, " ");
                StrUpper(break_request);
                // std::cout << "temp--> " << break_request <<'\n';
                // std::cout << "strcmp(break_request, 'CLOSE') --> "  << strcmp(break_request, "CLOSE") << '\n';
                std::string z(break_request);
                // std::cout << "z--> " << z <<'\n';
                z.erase(std::remove(z.begin(), z.end(), '\n'), z.end());
                std::cout << "z.compare(CLOSE) --> "<< (z.compare("CLOSE")) << '\n';
                if (z.compare("CLOSE") == 0) {
                        *connection_type = NONPERSISTENT;
                        std::cout << "connection set --> NONPERSISTENT " << *connection_type << '\n';
                } else {
                        *connection_type = PERSISTENT;
                        std::cout << "connection set --> PERSISTENT" << *connection_type << '\n';
                }
        }

        // free(&str_array);
        return 0;
}
// -----------------------------------------------------------------------
/*  Converts a string to upper-case  */
int StrUpper(char * buffer) {
        while ( *buffer ) {
                *buffer = toupper(*buffer);
                ++buffer;
        }
        return 0;
}

// -----------------------------------------------------------------------
void trim(std::string& s)
{
        size_t p = s.find_first_not_of(" \t");
        s.erase(0, p);

        p = s.find_last_not_of(" \t");
        if (std::string::npos != p)
                s.erase(p+1);
}
// -----------------------------------------------------------------------

int start_server() {

        int enable = 1;
        struct addrinfo hints, *server_info, *pointer;
        // struct sigaction sa; // NO IDEA WHY USED
        int getAddrRet;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; // USE MY IP
        //std::string port(PORT);
        if ((getAddrRet = getaddrinfo(NULL, PORT, &hints, &server_info)) != SUCCESS ) {
                std::cerr << "getaddrinfo error -->" << gai_strerror(getAddrRet) <<'\n';
                return EXIT_FAILURE;
        }

        // loop through all the results and bind to the first we can
        for (pointer = server_info; pointer != NULL; pointer = server_info->ai_next) {
                std::cout << "pointer family -->" << (pointer->ai_family == AF_INET ? "AF_INET" : "AF_INET6" ) << '\n';
                std::cout << "socket type --> " << pointer->ai_socktype << '\n';
                std::cout << "protocol --> " << pointer->ai_protocol << '\n';
                if ((listen_socket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol)) == FAILURE) {
                        std::perror("Socket Error ");
                        continue; // because we want to keep checking for other IP Addresses.
                }
                //-- CHECK
                if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
                        std::perror("setsockopt error");
                        return FAILURE;
                }
                //-- CHECK

                if (bind(listen_socket, pointer->ai_addr, pointer->ai_addrlen) == -1) {
                        close(listen_socket); // close the listening socket because the binding failed.
                        std::perror("server bind error");
                        continue;
                }
                break;
        }

        if (pointer == NULL) {
                std::perror("Server failed to bind");
                return FAILURE;
        }

        freeaddrinfo(server_info); // USE COMPLETE

        if (listen(listen_socket, BACKLOG) == FAILURE) {
                std::perror("listen error");
                return FAILURE;
        }
        // NO IDEA WHY USED - START
        // sa.sa_handler = sigchild_handler; // reap all dead processes
        // sigemptyset(&sa.sa_mask);
        // sa.sa_flags = SA_RESTART;
        // if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        //         std::perror("sigaction");
        //         exit(FAILURE);
        // }
        // NO IDEA WHY USED - END

        std::cout << "Address: " << pointer->ai_addr->sa_data << '\n';

        std::cout << "Server started. Waiting for the connection." << '\n';
        return SUCCESS;
}
