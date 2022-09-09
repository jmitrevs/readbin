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

num_read_t align(num_read_t unaligned) {
    const num_read_t alignment = 512;
    return (unaligned & ~(alignment - 1));
}

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

int main(int ac, char** av) {

    try {

        std::string infile;
        std::string outfile;
        std::string xclbin_file;
        std::string dev_id;

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


        // std::vector<uint8_t, aligned_allocator<uint8_t>> readbuf(READ_SIZE)
        // std::vector<writebuf_t, aligned_allocator<writebuf_t>> writebuf(NUM_CHANNELS)

        constexpr size_t READBUF_SIZE = READ_SIZE * sizeof(uint8_t);
        constexpr size_t WRITEBUF_SIZE = NUM_CHANNELS * sizeof(writebuf_t);

        std::cout << "READBUF_SIZE = " << std::hex << READBUF_SIZE << ", WRITEBUF_SIZE = " << WRITEBUF_SIZE << std::endl;

        // open files
        fileHelper fhin(infile, O_RDONLY | O_DIRECT);
        fileHelper fhout(outfile, O_CREAT | O_WRONLY | O_DIRECT, 0644);

        // Allocate Buffer in Global Memory
        cl_mem_ext_ptr_t inExt = {0};
        cl_mem_ext_ptr_t outExt = {0};
        inExt.flags = XCL_MEM_EXT_P2P_BUFFER;
        outExt.flags = XCL_MEM_EXT_P2P_BUFFER;

        static uint8_t readbuf[READ_SIZE];
        static writebuf_t channels[NUM_CHANNELS]

        num_read_t filein_offset = 0;
        num_read_t fileout_offset = 0;

        while(1) {

            num_read_t filein_offset_aligned = align(filein_offset);
            num_read_t rel_offset = filein_offset - filein_offset_aligned;

            std::cout << "aligned read offset (hex) " << std::hex << filein_offset_aligned << ", rel_offset = " << rel_offset << std::endl;

            auto numread = pread(fhin.fd(), readbuf, READBUF_SIZE, filein_offset_aligned);
            std::cout << "numread = " << std::hex << numread <<  std::endl;
            if (numread < rel_offset) {
                std::cerr << "ERR: pread failed: "
                            << " error: " << errno << ", " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            } else if (numread == rel_offset) {
                exit(EXIT_SUCCESS);
            }

            num_read_t numReadIn;
            num_read_t numWrittenOut;

            process_data(readbuf, numread, rel_offset, &numReadIn, channels, &numWrittenOut);
            //OCL_CHECK(err, err = q.enqueueUnmapBuffer(buffer_input, p2p_in));

            auto numWritten = numWrittenOut * sizeof(writebuf_t);

            OCL_CHECK(err, void* p2p_out = q.enqueueMapBuffer(buffer_output,                      // buffer
                                                              CL_TRUE,                    // blocking call
                                                              CL_MAP_WRITE | CL_MAP_READ,                // Indicates we will be writing
                                                              0,                          // buffer offset
                                                              WRITEBUF_SIZE,          // size in bytes
                                                              nullptr, nullptr,
                                                              &err)); // error code

            std::cout << "numWritten: " << std::hex << numWritten << ", fileout_offset: " << fileout_offset << std::endl;
            auto numActuallyWritten = pwrite(fhout.fd(), p2p_out, WRITEBUF_SIZE, fileout_offset);
            if (numActuallyWritten < 0) {
                std::cerr << "ERR: pwrite failed: "
                            << " error: " << errno << ", " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            } else if (numActuallyWritten != WRITEBUF_SIZE) {
                std::cerr << "ERR: pwrite failed: "
                            << numActuallyWritten << "  bytes written, but asked for " << numWritten << std::endl;
                exit(EXIT_FAILURE);
            }
            //OCL_CHECK(err, err = q.enqueueUnmapBuffer(buffer_output, p2p_out));
            filein_offset = filein_offset_aligned + numReadIn;
            fileout_offset += WRITEBUF_SIZE;
            std::cout << "Next iteration with read offset (hex) " << std::hex << filein_offset
                << " and write offset (hex) " << fileout_offset << std::endl;
        }
    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

}
