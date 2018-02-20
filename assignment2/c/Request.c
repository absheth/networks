#include <stdio.h>
#include <sys/time.h>
#include "Request.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

char * working_directory;

void name(/* arguments */) {
        /* code */
}
void initialize_request(Request *p_request) {
        printf("<-- Initializing --> \n");
        working_directory = getenv("PWD");
}
/*
   int read_request(int connFD, Request *p_request) {
        int duration = 5; // Seconds
        char buffer[MAXBUFFERSIZE];
        int rval;

        fd_set fds; // Array of bits // One bit for one file descriptor
        struct timeval timeout;

        // Set timeout to 10 seconds
        timeout.tv_sec = duration; // Seconds
        timeout.tv_usec = 0; // Microseconds
        int get_found = 0;

        do {
                //Reset file descriptor set
                FD_ZERO(&fds); // Clear all the bits
                FD_SET(connFD, &fds); // Set bit for connFD

                //Wait until the time out is ready
                rval = select(connFD+1, &fds, NULL, NULL, &timeout);
                if (rval < 0) {
                        // Error calling select()
                        quit("Error while calling select()");

                } else if (rval == 0) {
                        //time out
                        return FAILURE;
                } else {
                        readline(connFD, buffer, MAXBUFFERSIZE - 1);
                        Trim(buffer);
                        if (buffer[0] == '\0') {
                                break;
                        }
                        parsebuffer(connFD, buffer,p_request);
                        printf("buffer --> %s\n", buffer);
                        // printf("working_directory --> %s\n", working_directory);

                }

        } while(1);

        return SUCCESS;
   }
 */

void quit(char *p_error_message) {

        fprintf(stderr, "p_error_message --> %s\n", p_error_message);
        exit(EXIT_FAILURE);
}

/*  Read a line from a socket  */

ssize_t readline(int connFD, void *vptr, size_t maxlen) {

        ssize_t n, rc;
        char c, *buffer;

        buffer = (char*)vptr;

        for ( n = 1; n < maxlen; n++ ) {

                if ( (rc = read(connFD, &c, 1)) == 1 ) {
                        printf("req_len --> %c\n", c);
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

int parsebuffer(int connFD, char *buffer, Request *p_request) {
        printf("connFD --> %d \n", connFD);
        printf("buffer --> %s\n", buffer);

        return SUCCESS;
}

// Local methods
/*  Removes trailing whitespace from a string  */

int Trim(char * buffer) {
        int n = strlen(buffer) - 1;

        while ( !isalnum(buffer[n]) && n >= 0 )
                buffer[n--] = '\0';

        return 0;
}
