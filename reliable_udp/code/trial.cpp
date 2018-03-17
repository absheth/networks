#include <iostream>
#include <sstream>
struct pkt {
    int seqnum;
    int acknum;
    int checksum;
    char payload[26];
};

int main(int argc, char **argv) {
    std::cout << std::endl;
    std::cout << "size of char --> " << sizeof(char) << std::endl;
    std::cout << "size of int --> " << sizeof(int) << std::endl;
    // std::stringstream ss;
    // ss << "Akash Sheth";
    // std::cout << ss.str() << " and length is " << ss.str().length() << std::endl;
    struct pkt packet;
    packet.seqnum = 1;
    packet.acknum = 1;
    std::string x = "Akash Sheth is a good boy";
    std::cout << "x.length --> "<< x.length() << std::endl;
    
    strncpy(packet.payload, x.c_str(), sizeof(packet.payload));
    std::cout << "Packet.seqnum --> " << packet.seqnum << std::endl;
    std::cout << "Packet.payload --> " << packet.payload << " | Length --> "
              << strlen(packet.payload) << " | size: " << sizeof(packet.payload) << std::endl;
    std::cout << "Packet.acknum --> " << packet.acknum << std::endl;

    return 0;
}
