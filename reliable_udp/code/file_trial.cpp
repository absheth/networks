#include <fstream>
#include <iostream>

int main() {
    char filename[] = "test.txt";
    std::ofstream outputfile;
    int sampledata;

    // creating, opening and wrinting/appending data into a file.

    outputfile.open(filename, std::fstream::out|std::fstream::app);

    // Simple error handling for file creating/opening for writing,
    // test if fail to open the file.
    if (outputfile.fail()) {
        std::cout << "Creating and opening file " << filename << " for writing " << std::endl;
        std::cout << "The " << filename << " file could not be created/opened." << std::endl;
        std::cout << std::endl;
        std::cout << "Possible errors: " << std::endl;
        std::cout << "1. The file does not exists." << std::endl;
        std::cout << "2. The path was not found." << std::endl;

    } else {
        std::cout << "The " << filename << " was created and opened successfully. " << std::endl;
        std::cout << "Do some file (re)writing." << std::endl;
        outputfile << "Writing data to this file " << std::endl;
        outputfile << "--------------------------" << std::endl;

        std::cout << "Check the " << filename << "file contents" << std::endl;
        std::cout << "If the file already has data then the new data will be appended to it."
                  << std::endl;
        
        // Writing some integers to the file.
        for (sampledata = 0; sampledata <=10; sampledata++) {
            outputfile << sampledata << " "; 
            outputfile << std::endl;
        }

        // close the output file.
        outputfile.close();

        if (outputfile.fail()) {
            std::cout << "The " << filename << " file could not be closed." << std::endl;
            std::cout << std::endl;
            exit(1);
        } else {
            std::cout << "" << std::endl;
            std::cout << "The " << filename << " file closed successfully." << std::endl;
        }
    }
    return 0;
}
