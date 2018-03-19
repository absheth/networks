#include <climits>
#include <cstring>
#include <iostream>
#include <iterator>
#include <list>
#include <sstream>
int chartoint(char* p_charstream);
void inttochar(char* p_char, int p_number);
struct pkt {
    int seqnum;
    int acknum;
    int checksum;
    char payload[26];
};

int main(int argc, char** argv) {
    /* std::cout << std::endl;
    std::cout << "size of char --> " << sizeof(char) << std::endl;
    std::cout << "size of int --> " << sizeof(int) << std::endl;
    // std::stringstream ss;
    // ss << "Akash Sheth";
    // std::cout << ss.str() << " and length is " << ss.str().length() << std::endl;
    struct pkt packet;
    packet.seqnum = 1;
    packet.acknum = 1;
    std::string x = "Akash Sheth";
    std::cout << "x.length --> " << x.length() << std::endl;
    strncpy(packet.payload, x.c_str(), sizeof(packet.payload));
    std::cout << "Packet.seqnum --> " << packet.seqnum << std::endl;
    std::cout << "Packet.payload --> " << packet.payload << " | Length --> "
              << strlen(packet.payload) << " | size: " << sizeof(packet.payload) << std::endl;
    std::cout << "Packet.acknum --> " << packet.acknum << std::endl;
    std::cout << "MAX INT --> " << UINT_MAX << std::endl;
    */
    /*
    unsigned char bytes[4];
    bytes[0] = (n >> 24) & 0xFF;
    bytes[1] = (n >> 16) & 0xFF;
    bytes[2] = (n >> 8) & 0xFF;
    bytes[3] = n & 0xFF;
    printf("%x %x %x %x\n", bytes[0], bytes[1], bytes[2], bytes[3]);

    unsigned int y;
    y = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    std::cout << y << std::endl;
    */

    //***************************************************
    /*char buffer[20];
    // unsigned long n = UINT_MAX - 1;
    unsigned long n = 54;
    std::cout << std::endl;
    std::cout << "Number to be converted --> " << n << std::endl;
    char char_ptr[4];
    memset(char_ptr, 0, 4);
    inttochar(&char_ptr[0], n);
    printf("Converted to 4 chars: %x %x %x %x\n", char_ptr[0], (unsigned char)char_ptr[1],
    char_ptr[2], char_ptr[3]);

    std::cout << "char_ptr --> " << char_ptr << std::endl;
    std::cout << "AKASH --> " << char_ptr[0] << char_ptr[1]<< char_ptr[2]<< char_ptr[3]<< std::endl;
    unsigned int z = chartoint(&char_ptr[0]);
    std::cout << "Converted back to int --> " << z << std::endl;
    memcpy(buffer, char_ptr, 4);

    buffer[0] = char_ptr[0];
    buffer[1] = char_ptr[1];
    buffer[2] = char_ptr[2];
    buffer[3] = char_ptr[3];
    buffer[4] = 'a';
    buffer[5] = 'k';
    buffer[6] = 'a';
    buffer[7] = 's';
    buffer[8] = 'h';
    buffer[9] = ' ';
    buffer[10] = 's';
    buffer[11] = 'h';
    buffer[12] = 'e';
    buffer[13] = 't';
    buffer[14] = 'h';
    std::cout << "buffer --> " << buffer << std::endl;
    for (int i = 4; i < 14; i++) {
        std::cout << "buffer "<< i << " --> " << buffer[i] << std::endl;

    }
    unsigned int q = chartoint(&buffer[0]);
    std::cout << "Conveted back to int --> " << q << std::endl;
    char p = 'a';
    unsigned char r = 'a';
    std::cout << "p --> " << (int)p << std::endl;
    std::cout << "p --> " << (int)(char)r << std::endl;

    std::stringstream ss;
    ss << buffer;
    // ss << char_ptr[1];
    // ss << char_ptr[2];
    // ss << char_ptr[3];
    std::cout << "ss -->" << ss.str().c_str() <<" length "<< strlen(ss.str().c_str()) << std::endl;
    // char temp = '?';
    //
    // printf("%x\n",(char)char_ptr[3]);
    // printf("%x\n",(unsigned char)(char)char_ptr[3]);*/
    int a = 3;
    if (a) {
        std::cout << "TRUE for " << a << std::endl;

    } else {
        std::cout << "FALSE for " << a << std::endl;
    }
    std::list<unsigned int> listOfNumbers;
    // Inserting elements at end in list
    listOfNumbers.push_back(5);
    listOfNumbers.push_back(6);

    // Inserting elements at front in list
    listOfNumbers.push_front(2);
    listOfNumbers.push_front(1);

    // Iterating over list elements and display them
    std::list<unsigned int>::iterator it = listOfNumbers.begin();
    while (it != listOfNumbers.end()) {
        std::cout << (*it) << " ";
        it++;
    }
    std::cout << std::endl;

    // Inserting elements in between the list using
    // insert(pos,elem) member function. Let's iterate to
    // 3rd position
    it = listOfNumbers.begin();
    it++;
    it++;
    // Iterator 'it' is at 3rd position.
    listOfNumbers.insert(it, 4);
    int index = std::distance(listOfNumbers.begin(), it);
    std::cout << "index --> " << index << std::endl;
    
    return 0;
}

// -----------------------------------------------------------------------
/* Convert unsigned char array to int */
int chartoint(char* p_charstream) {
    return ((unsigned char)p_charstream[0] << 24) | ((unsigned char)p_charstream[1] << 16) |
           ((unsigned char)p_charstream[2] << 8) | (unsigned char)p_charstream[3];
}
/* Convert unsigned into char array of 4 bytes */
void inttochar(char* p_char, int p_number) {
    unsigned char x;
    // x = 75 & 0xFF;
    // printf("INTTOCHAR --> %x", x);
    // char l = x;
    // printf("here --> %d %c ", int(l), l);
    // std::cout << std::endl;
    x = (p_number >> 24) & 0xFF;
    p_char[0] = x;
    x = (p_number >> 16) & 0xFF;
    p_char[1] = x;
    x = (p_number >> 8) & 0xFF;
    p_char[2] = x;
    x = p_number & 0xFF;
    p_char[3] = x;

    // p_char[0] = char((p_number >> 24) & 0xFF);
    // p_char[1] = char((p_number >> 16) & 0xFF);
    // p_char[2] = char((p_number >> 8) & 0xFF);
    // p_char[3] = char(p_number & 0xFF);
}
// -----------------------------------------------------------------------
