// #include <iostream>
#include <stdio.h>

// #define MAXBUFFERSIZE 1024
#define SUCCESS 0
#define FAILURE -1
#ifndef REQUEST_H
#define REQUEST_H


typedef struct Request{
        char *file;
        int status;
}Request;

void initialize_request(Request *p_request);
int read_request(int connFD, Request *p_request);
int parse_header(char *buffer, Request *p_request);
void quit(char *message);
ssize_t readline(int sockd, void *vptr, size_t maxlen);
int Trim(char * buffer);
int parsebuffer(int connFD, char *buffer, Request *p_request);
/*
class HttpRequest {
private:
        std::string file;
        int status;
public:
        void initialize_request();
        int read_request(int connFD);
        int parse_header(char *buffer);
        void quit(std::string p_error_message);
        ssize_t readline(int sockd, void *vptr, size_t maxlen);
};
*/
#endif
