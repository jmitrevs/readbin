// This dumps the binary file produced by the code into text

#include <iostream>
#include <fstream>
#include "ap_fixed.h"
#include "cnn/firmware/defines.h"
using writebuf_t = result_t::value_type;

int main(int ac, char** av) {
    if (ac != 2) {
        std::cerr << "Usage: " << av[0] << " infile\n\toutput is streamed on stdout\n";
        return 1;
    }

    std::ifstream infile(av[1], std::ios::in | std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Could not open " << av[1] << std::endl;
        return 1;
    }

    writebuf_t val;
    while (infile.read(reinterpret_cast<char*>(&val), sizeof(writebuf_t))) {
        std::cout << val << std::endl;
    }
    return 0;
}
    
