// Header file for input output functions
#include <iostream>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;
int main(int argc, char const *argv[]) {

        char clientData[] = "Akash Sheth is a good boy.";
        int nSocket;
        nSocket = socket(AF_INET, SOCK_STREAM, 0);

        //specify an address for the socket
        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(9158);
        server_address.sin_addr.s_addr = INADDR_ANY;

        bind(nSocket, (struct sockaddr*)&server_address, sizeof(server_address));
        listen(nSocket, 5);
        int client_socket;
        client_socket = accept(nSocket, NULL, NULL);
        // check for error with the connection

        send(client_socket, &clientData, sizeof(clientData), 0);

        close(nSocket);
        return 0;
}
