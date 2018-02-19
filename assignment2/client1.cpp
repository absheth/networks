// Header file for input output functions
#include <iostream> //
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;
int main(int argc, char const *argv[]) {
        int nSocket;
        nSocket = socket(AF_INET, SOCK_STREAM, 0);

        //specify an address for the socket
        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(9158);
        server_address.sin_addr.s_addr = INADDR_ANY;

        int conn_status = connect(nSocket, (struct sockaddr*)&server_address, sizeof(server_address));
        // check for error with the connection
        if (conn_status == -1) {
                std::cerr << "Error making connection with the remote socket." << '\n';
        }
        char server_response[1024];
        recv(nSocket, &server_response, sizeof(server_response), 0);

        //Print the server response.
        std::cout << "Response from server --> " << server_response<< '\n';
        close(nSocket);
        return 0;
}
