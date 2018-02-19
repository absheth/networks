// string::substr
#include <iostream>
void increment(int *x);
int Trim(char * buffer) {
    int n = strlen(buffer) - 1;

    while ( !isalnum(buffer[n]) && n >= 0 )
	buffer[n--] = '\0';

    return 0;
}


int main ()
{
        std::string str="Akash Sheth";
        std::size_t pos = str.find("Sheth"); // position of "live" in str
        std::cout << "postion --> " << pos << '\n';

        int x = 0;

        std::cout << (!x ? "FALSE":"TRUE") << '\n';
        char name[] = "Akash Sheth";
        Trim(name);
        std::cout << name << " is a good boy" << '\n';
        // std::string str3 = str.substr (pos); // get from "live" to the end

        // std::cout << str2 << ' ' << str3 << '\n';
        increment(&x);
        std::cout << "x --> " << x << '\n';
        increment(&x);
        std::cout << "x --> " << x << '\n';
        increment(&x);
        std::cout << "x --> " << x << '\n';
        return 0;
}

void increment(int *x) {
        (*x)++;

}
