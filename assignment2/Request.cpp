#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include "Request.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
int Trim(char * buffer);
std::string working_directory = getenv("PWD");
void HttpRequest::initialize_request() {
        std::cout << this->file.empty() << '\n';
}

int HttpRequest::read_request(int connFD) {
        int duration = 5; // Seconds
        char buffer[MAXBUFFERSIZE];
        int rval;

        fd_set fds; // Array of bits // One bit for one file descriptor
        struct timeval timeout;

        /* Set timeout to 10 seconds */
        timeout.tv_sec = duration; // Seconds
        timeout.tv_usec = 0; // Microseconds
        
        do {
                /* Reset file descriptor set */
                FD_ZERO(&fds); // Clear all the bits
                FD_SET(connFD, &fds); // Set bit for connFD

                /* Wait until the time out is ready */
                rval = select(connFD+1, &fds, NULL, NULL, &timeout);
                if (rval < 0) {
                        // Error calling select()
                        quit("Error while calling select()");

                } else if (rval == 0) {
                        /* time out */
                        return FAILURE;
                } else {
                        readline(connFD, buffer, MAXBUFFERSIZE - 1);
                        Trim(buffer);
                        if (buffer[0] == '\0') {
                                break;
                        }
                        // printf("buffer --> %s\n", buffer);
                        std::cout << "buffer --> " << buffer <<'\n';
                        std::cout << "working_directory --> " << working_directory << '\n';
                }

        } while(1);

        return SUCCESS;
}

void HttpRequest::quit(std::string p_error_message) {
        std::cerr << "error message " << p_error_message << '\n';
        exit(EXIT_FAILURE);
}

/*  Read a line from a socket  */

ssize_t HttpRequest::readline(int connFD, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char    c, *buffer;

    buffer = (char*)vptr;

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

// Local methods
/*  Removes trailing whitespace from a string  */

int Trim(char * buffer) {
    int n = strlen(buffer) - 1;

    while ( !isalnum(buffer[n]) && n >= 0 )
	buffer[n--] = '\0';

    return 0;
}
