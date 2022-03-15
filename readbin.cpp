// An example of parsing the binary file

// NOTE:  this assumes a little-endian architecture

#include <unistd.h>
#include <fcntl.h>
#include <exception>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include "readbin.h"

// Use a class for file acces for RAII
class fileHelper {
public:
    fileHelper(std::string filename, int oflag, int permission = 0);
    ~fileHelper();
    int fd() { return m_fd; };
private:
    int m_fd;
};

fileHelper::fileHelper(std::string filename, int oflag, int permission) {
    if (permission == 0) {
	m_fd = open(filename.c_str(), oflag);
    } else {
	m_fd = open(filename.c_str(), oflag, permission);
    }   
    if (m_fd < 0) {
        std::stringstream ss;
        ss << "Could not open " << filename;
        throw std::invalid_argument(ss.str());
    }
}

fileHelper::~fileHelper() {
    if (m_fd >= 0) {
        close(m_fd);
    }
}

static uint8_t readbuf[READ_SIZE];
static uint8_t writebuf[WRITE_SIZE];


int main(int ac, char** av) {

    try {

        std::string infile;
        std::string outfile;

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce help message")
            ("input-file,i", po::value<std::string>(&infile), "input file")
            ("output-file,o", po::value<std::string>(&outfile), "output file")
            ;

        po::positional_options_description p;
        p.add("input-file", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(ac, av).
                  options(desc).positional(p).run(), vm);
        po::notify(vm);


        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        if (!vm.count("input-file")) {
            std::cerr << "input-file not given." << std::endl;
            return 1;
        }

        if (!vm.count("output-file")) {
            std::cerr << "output-file not given." << std::endl;
            return 1;
        }
        
        fileHelper fhin(infile, O_RDONLY); // | O_DIRECT);

        fileHelper fhout(outfile, O_CREAT | O_WRONLY, 0644); // | O_DIRECT);

        // Now process the files
        off_t inoff = 0;
        off_t outoff = 0;
        auto numread = pread(fhin.fd(), static_cast<void *>(readbuf), READ_SIZE, inoff);

        while (true) {
            // std::cout << "numread = " << numread << ", inoff = " << inoff << ", outoff = " << outoff << std::endl;
            if (numread < 0) {
                std::cerr << "ERR: pread failed: "
                          << " error: " << errno << ", " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            } else if (numread == 0) {
                break;
            }
            off_t numReadIn;
            off_t numWritten;
            process(readbuf, writebuf, numread, numReadIn, numWritten);
            // std::cout << "numReadIn = " << numReadIn << ", numWritten = " << numWritten << std::endl;
            auto numActuallyWritten = pwrite(fhout.fd(), static_cast<void *>(writebuf), numWritten, outoff);
            if (numActuallyWritten < 0) {
                std::cerr << "ERR: pwrite failed: "
                          << " error: " << errno << ", " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            } else if (numActuallyWritten != numWritten) {
                std::cerr << "ERR: pwrite failed: "
                          << numActuallyWritten << "  bytes written, but asked for " << numWritten << std::endl;
                exit(EXIT_FAILURE);
            }  
            inoff += numReadIn;
            outoff += numWritten;
            numread =  pread(fhin.fd(), static_cast<void *>(readbuf), READ_SIZE, inoff);
        }
    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

}
