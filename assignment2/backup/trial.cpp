// string::substr
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
std::vector<std::string> split(const std::string &s, char delim);
void increment(int *x);
void trim(std::string& s);
int Trim(char * buffer) {
        int n = strlen(buffer) - 1;

        while ( !isalnum(buffer[n]) && n >= 0 )
                buffer[n--] = '\0';

        return 0;
}


int main ()
{
        //int a =("CLOSE" == "CLOSE");
        std::string z = "CLOSE";
        std::cout << z.compare("CLOSE") << '\n';
        std::string str="Akash Sheth";
        std::size_t pos = str.find("Sheth"); // position of "live" in str
        std::cout << "postion --> " << pos << '\n';

        int x = 0;

        std::cout << (!x ? "FALSE" : "TRUE") << '\n';
        char name[] = "Akash Sheth";
        Trim(name);
        std::cout << &name[strlen(name)-2] << '\n';
        if ((strcmp(&name[strlen(name)-2], "th") == 0)) {
                std::cout << "TRUE" << '\n';
        }
        std::cout << (strcmp(&name[strlen(name)-2], "t") == 0) << '\n';


        std::cout << "length --> " << strlen(name) << '\n';
        std::cout << name << " is a good boy" << '\n';
        // std::string str3 = str.substr (pos); // get from "live" to the end

        // std::cout << str2 << ' ' << str3 << '\n';
        increment(&x);
        std::cout << "x --> " << x << '\n';
        increment(&x);
        std::cout << "x --> " << x << '\n';
        increment(&x);
        std::cout << "x --> " << x << '\n';


        std::string temp_str = "   Connection    :   Keep-Alive    ";
        std::cout << "temp_str --> " << temp_str << '\n';
        // std::string myText("some-text-to-tokenize");
        // std::istringstream iss(temp_str);
        // std::string token;
        // while (std::getline(iss, token, ':'))
        // {
        //         trim(token);
        //         std::cout << token << std::endl;
        // }
        char str_array[9999];
        strcpy(str_array, temp_str.c_str());
        char * break_request = strtok(str_array, ":");

        std::string y(break_request);
        trim(y);
        std::cout << "y --> " << y <<'\n';
        //strcmpi(y.c_str(), "connection");
        //
        // char str1[] ="- This, a sample string.";
        // char * pch;
        // printf ("Splitting string \"%s\" into tokens:\n",str1);
        // pch = strtok (str1," ,.-");
        // while (pch != NULL)
        // {
        //         printf ("%s\n",pch);
        //         pch = strtok (NULL, " ,.-");
        // }

        std::string foo = "whatever";
        std::string bar = "whatever";

        if(foo == bar){
                std::cout << "they are equal";
        }

        return 0;
}

void increment(int *x) {
        (*x)++;

}


std::vector<std::string> split(const std::string &s, char delim) {
        std::stringstream ss(s);
        std::string item;
        std::vector<std::string> tokens;
        while (getline(ss, item, delim)) {
                tokens.push_back(item);
        }
        return tokens;
}

void trim(std::string& s)
{
        size_t p = s.find_first_not_of(" \t");
        s.erase(0, p);

        p = s.find_last_not_of(" \t");
        if (std::string::npos != p)
                s.erase(p+1);
}
