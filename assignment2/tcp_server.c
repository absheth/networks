// Header file for input output functions
#include <stdio.h>
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


void getIPAddr();
void sigchild_handler(int s);
int serviceRequest(int connFD);
void *get_in_addr(struct sockaddr *sa);
void quit(char *p_error_message);
int Trim(char * buffer);
ssize_t Writeline(int sockd, const void *vptr, size_t n);
ssize_t readline(int connFD, void *vptr, size_t maxlen);
int parse_request(char *buffer, char *http_req, int* count, int* connection_type);
int StrUpper(char * buffer);
#define PORT "9158"  // the port users will be connecting to
#define SUCCESS 0
#define FAILURE -1
#define BACKLOG 5
#define MAXBUFFERSIZE 1024
#define PERSISTENT 1
#define NONPERSISTENT 2
char * working_directory;
int main(int argc, char const *argv[]) {

        printf("argc --> %d\n", argc);
        working_directory = getenv("PWD");
        printf("%s\n", working_directory);
        // getIPAddr();
        // -----------------------------------------------------------------------
        int enable = 1;
        int listen_socket, communicate_socket;  // listen on sock_fd, new connection on new_fd
        struct addrinfo hints, *server_info, *pointer;
        struct sockaddr_storage clientAddr; // This stores the client address information.
        socklen_t sin_size;
        struct sigaction sa; // NO IDEA WHY USED
        int getAddrRet;
        char clientIP[INET6_ADDRSTRLEN];
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM; //SOCK_STREAM, SOCK_DGRAM
        hints.ai_flags = AI_PASSIVE; // USE MY IP
        if ((getAddrRet = getaddrinfo(NULL, PORT, &hints, &server_info)) != SUCCESS ) {
                fprintf(stderr, "getaddrinfo error --> %s\n", gai_strerror(getAddrRet));
                return 1; // CHECK WHY 1;
        }
        // loop through all the results and bind to the first we can
        for (pointer = server_info; pointer != NULL; pointer = server_info->ai_next) {
                printf("pointer family --> %s\n", (pointer->ai_family == AF_INET ? "AF_INET" : "AF_INET6" ));
                printf("socket type --> %d\n", pointer->ai_socktype);
                printf("protocol --> %d\n", pointer->ai_protocol); // 6 because TCP, 17 because UDP

                if ((listen_socket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol)) == FAILURE) {
                        perror("## Socket Error ##");
                        continue; // because we want to keep checking for other IP Addresses.
                }
                if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
                        perror("setsockopt error");
                        exit(1);
                }


                if (bind(listen_socket, pointer->ai_addr, pointer->ai_addrlen) == -1) {
                        close(listen_socket);
                        perror("server bind error");
                        continue;
                }
                break;
        }

        if (pointer == NULL) {
                fprintf(stderr, "Server failed to bind\n");
                return 2;
        }
        freeaddrinfo(server_info); // USE COMPLETE

        if (listen(listen_socket, BACKLOG) == FAILURE) {
                perror("listen error");
                exit(1);
        }
        // NO IDEA WHY USED - START
        sa.sa_handler = sigchild_handler; // reap all dead processes
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                perror("sigaction");
                exit(1);
        }
        // NO IDEA WHY USED - END
        printf("Server started. Waiting for the connection.\n");
        int count = 0;
        while (1) {
                sin_size = sizeof clientAddr;
                communicate_socket = accept(listen_socket, (struct sockaddr *)&clientAddr, &sin_size);
                if (communicate_socket == FAILURE) {
                        perror("Accept error");
                        continue;
                }

                printf("Communication file descriptor --> %d\n", communicate_socket);
                inet_ntop(clientAddr.ss_family, get_in_addr((struct sockaddr *)&clientAddr), clientIP, sizeof clientIP);

                printf("Connection from --> %s\n", clientIP);


                // pthreads
                //if (!fork()) { // This is a child process;
                close(listen_socket); // Child doesn't need the listener
                serviceRequest(communicate_socket);
                // if (send(communicate_socket, "Akash Sheth is a good boy.", sizeof "Akash Sheth is a good boy.", 0) == FAILURE) {
                //         perror("send error");
                // }
                shutdown (communicate_socket, SHUT_RDWR);
                close(communicate_socket);
                printf("******HERE******\n");
                printf("count--> %d\n", count++);
                exit(0); // for process
                // }

                // if (count > 0) {
                //         break;
                // }

                close(communicate_socket); // Parent does not need this.
        }

        // -----------------------------------------------------------------------

        return 0;
}

// -----------------------------------------------------------------------
void sigchild_handler(int s)
{
        while(waitpid(-1, NULL, WNOHANG) > 0) ;
}
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
int serviceRequest(int connFD) {
        printf("connFD ---> %d\n", connFD);
        int duration = 5; // Seconds
        char receive_buffer[MAXBUFFERSIZE];
        char send_buffer[MAXBUFFERSIZE];
        char http_req[MAXBUFFERSIZE];
        int rval, file_desc, nRead;
        int req_len = 0;
        char filepath[9999];
        int connection_type = 10;
        fd_set fds; // Array of bits // One bit for one file descriptor
        struct timeval timeout;

        /* Set timeout to 10 seconds */
        timeout.tv_sec = duration; // Seconds
        timeout.tv_usec = 0; // Microseconds
        /* Reset file descriptor set */
        FD_ZERO(&fds); // Clear all the bits
        FD_SET(connFD, &fds); // Set bit for connFD

        /* Wait until the time out is ready */
        rval = select(connFD+1, &fds, NULL, NULL, &timeout);
        //printf("rval --> %d\n", rval);
        int count = 0;
        do {
                /* code */

                if (rval < 0) {
                        // Error calling select()
                        quit("Error while calling select()");

                } else if (rval == 0) {
                        fprintf(stderr, "TIMEOUT\n");
                        return -1;
                } else {
                        // req_len = recv(connFD, receive_buffer, MAXBUFFERSIZE - 1, 0);

                        req_len = readline(connFD, receive_buffer, MAXBUFFERSIZE - 1);
                        // printf("req_len --> %d\n", req_len);
                        Trim(receive_buffer);
                        parse_request(receive_buffer, http_req, &count, &connection_type);
                        // if (receive_buffer[0] == '\0') {
                        //         break;
                        // }
                        // printf("working_directory --> %s\n", working_directory);

                }
        } while(connection_type == 10);
        printf("receive_buffer --> %s\n", receive_buffer);
        printf("receive_buffer size --> %lu\n", sizeof receive_buffer);
        printf("req_len --> %d\n", req_len);
        printf("CONNECTION TYPE %s\n", connection_type==PERSISTENT?"PERSISTENT":"NONPERSISTENT");
        // int i;
        // for (i = 0; i< req_len; i++) {
        //         printf("%d --> %c\n", i, receive_buffer[i]);
        // }
        char *break_requst[3];
        break_requst[0] = strtok(http_req, " ");
        printf("break_requst[0] %s\n", break_requst[0]);
        if (strcmp(break_requst[0], "GET") == 0) {
                break_requst[1] = strtok(NULL, " ");
                break_requst[2] = strtok(NULL, " ");
                printf("1 --> %s | 2 --> %s\n", break_requst[1], break_requst[2]);
                if (strcmp(break_requst[2], "HTTP/1.0") != 0 && strcmp(break_requst[2], "HTTP/1.1") != 0) {

                        write(connFD, "HTTP/1.0 400 Bad Request", 25);
                } else {
                        strcpy(filepath, working_directory);
                        printf("Directory --> %s\n", filepath);
                        strcpy(&filepath[strlen(working_directory)], break_requst[1]);
                        printf("FULL PATH --> %s\n", filepath);
                        if ( (file_desc=open(filepath, O_RDONLY))!=-1 )    //FILE FOUND
                        {
                                // printf("file_desc --> %d\n", file_desc);
                                send(connFD, "HTTP/1.0 200 OK\n\n", 17, 0);
                                while ( (nRead=read(file_desc, send_buffer, MAXBUFFERSIZE))>0 ) {
                                        // printf("%s\n", send_buffer);
                                        //printf("%d\n", nRead);
                                        send (connFD, send_buffer, nRead, 0);
                                }
                                // send (connFD,"done.", 5, 1);
                        }
                        else  {
                                printf("file_desc --> %d\n", file_desc);
                                printf("NOT FOUND\n");
                                write(connFD, "HTTP/1.0 404 Not Found\n", 23);            //FILE NOT FOUND

                                char buffer[100];

                                sprintf(buffer, "<HTML>\n<HEAD>\n<TITLE>Server Error %d</TITLE>\n"
                                        "</HEAD>\n\n", 404);
                                Writeline(connFD, buffer, strlen(buffer));

                                sprintf(buffer, "<BODY>\n<H1>Server Error: %d </H1>\n", 404);
                                Writeline(connFD, buffer, strlen(buffer));
                                sprintf(buffer, "<BODY>\n<H1>Server Message: %s </H1>\n", "File not found");
                                Writeline(connFD, buffer, strlen(buffer));
                                sprintf(buffer, "<P>The request could not be completed.</P>\n"
                                        "</BODY>\n</HTML>\n");
                                Writeline(connFD, buffer, strlen(buffer));

                                return 0;
                        }
                }

        } else {
                printf("NOT GET \n");
        }



        /*Request current_request;
           initialize_request(&current_request);
           if (read_request(connFD, &current_request) == FAILURE) {
                printf("FAILURE\n");

           } else {
                printf("SUCCESS\n");
           }*/
        return 0;
}
// -----------------------------------------------------------------------

void getIPAddr() {
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1) {
                perror("getifaddrs");
                exit(EXIT_FAILURE);
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
                family = ifa->ifa_addr->sa_family;

                if (family == AF_INET) {
                        s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                        host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                        if (s != 0) {
                                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                                exit(EXIT_FAILURE);
                        }
                        printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);
                }
        }
}


// -----------------------------------------------------------------------
/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n) {
        size_t nleft;
        ssize_t nwritten;
        const char *buffer;

        buffer = vptr;
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
        printf("IN READLINE\n");
        printf("MAXLEN --> %lu\n", maxlen);
        ssize_t n, rc;
        char c, *buffer;

        buffer = (char*)vptr;

        for ( n = 1; n < maxlen; n++ ) {

                if ( (rc = read(connFD, &c, 1)) == 1 ) {
                        // printf("rc--> %lu", rc);
                        // printf("req_len --> %c\n", c);
                        *buffer++ = c;
                        if ( c == '\n' )
                                break;
                }
                else if ( rc == 0 ) {
                        printf("rc--> %lu", rc);
                        if ( n == 1 )
                                return 0;
                        else
                                break;
                }
                else {
                        printf("rc--> %lu", rc);
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
void quit(char *p_error_message) {

        fprintf(stderr, "p_error_message --> %s\n", p_error_message);
        exit(EXIT_FAILURE);
}
// -----------------------------------------------------------------------
int parse_request(char *buffer, char *http_req, int* count, int* connection_type) {

        if (*count == 0) {
                // http_req = buffer;
                strcpy(http_req, buffer);
                (*count)++;

                printf("in if count--> %d\n", *count);
                printf("http--> %s\n", http_req);
                printf("buffer--> %s\n", buffer);

                return 0;
        }
        printf("http--> %s\n", http_req);
        printf("out if count--> %d\n", *count);
        char      *endptr;
        char      *temp;
        endptr = strchr(buffer, ':');
        temp = calloc( (endptr - buffer) + 1, sizeof(char) );

        strncpy(temp, buffer, (endptr - buffer));
        StrUpper(temp);
        buffer = endptr + 1;
        while ( *buffer && isspace(*buffer) )
                ++buffer;
        if ( *buffer == '\0' )
                return 0;

        //Checking for connection type.
        if ( !strcmp(temp, "CONNECTION") ) {
                StrUpper(buffer);
                printf("temp--> %s\n", temp);
                if (!strcmp(buffer, "CLOSE")) {
                        *connection_type = NONPERSISTENT;
                } else {
                        *connection_type = PERSISTENT;
                }
        }

        free(temp);
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
