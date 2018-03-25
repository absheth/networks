#include <unistd.h>
#include <chrono>
#include <iostream>

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << std::endl;
    std::cout << "---------------------------" << std::endl;
    // unsigned int sleep_time = 50;
    // usleep(sleep_time);
    // std::cout << "SLEEPING FOR " << sleep_time << " microseconds."<< std::endl;
    unsigned int sleep_time = 2;
    sleep(sleep_time);
    std::cout << "SLEEPING FOR " << sleep_time << " seconds. " << std::endl;
    std::cout << "---------------------------" << std::endl;
    std::cout << std::endl;

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";
    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    std::cout << "microseconds --> " << microseconds<< std::endl;
     
    return 0;
}
