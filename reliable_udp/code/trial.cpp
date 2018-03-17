#include <iostream>
#include <sstream>

int main(int argc, char **argv) {
    std::cout << std::endl;
    std::cout << "Number of arguments --> " << argc << std::endl;
    std::cout << "Argument --> " << argv[0] << std::endl;

    std::cout << "size of char --> " << sizeof(char) << std::endl;
    std::cout << "size of int --> " << sizeof(int) << std::endl;
    std::stringstream ss;
    ss << "Akash Sheth";
    std::cout << sizeof(ss)<< std::endl;
    ss << 1;
    std::cout << sizeof(ss)<< std::endl;
    ss <<"1";
    std::cout << sizeof(ss)<< std::endl;
    
    std::cout<< ss.str() << std::endl;
     
    
    
    return 0;
}
