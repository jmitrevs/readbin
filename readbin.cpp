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
#include "xcl2.hpp"
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

int main(int ac, char** av) {

    try {

        std::string infile;
        std::string outfile;
        std::string xclbin_file;
        std::string dev_id;

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce help message")
            ("xclbin-file,x", po::value<std::string>(&xclbin_file), "xclbin file")
            ("device,d", po::value<std::string>(&dev_id)->default_value("0"), "device id")
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

        // setup OpenCL stuff
        cl_int err;
        cl::Context context;
        cl::CommandQueue q;

        auto devices = xcl::get_xil_devices();
        // read_binary_file() is a utility API which will load the binaryFile
        // and will return the pointer to file buffer.
        auto fileBuf = xcl::read_binary_file(xclbin_file);
        cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
        cl::Program program;

        auto pos = dev_id.find(":");
        cl::Device device;
        if (pos == std::string::npos) {
            uint32_t device_index = stoi(dev_id);
            if (device_index >= devices.size()) {
                std::cout << "The device_index provided using -d flag is outside the range of "
                             "available devices\n";
                return EXIT_FAILURE;
            }
            device = devices[device_index];
        } else {
            if (xcl::is_emulation()) {
                std::cout << "Device bdf is not supported for the emulation flow\n";
                return EXIT_FAILURE;
            }
            device = xcl::find_device_bdf(devices, dev_id);
        }

        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
        std::cout << "Trying to program device[" << dev_id << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        program = cl::Program(context, {device}, bins, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << dev_id << "] with xclbin file!\n";
            exit(EXIT_FAILURE);
        } else
            std::cout << "Device[" << dev_id << "]: program successful!\n";

        // Now do buffers

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

        OCL_CHECK(err, cl::Buffer buffer_input(context, CL_MEM_READ_ONLY | CL_MEM_EXT_PTR_XILINX, READBUF_SIZE, &inExt,
                                               &err));
        OCL_CHECK(err, cl::Buffer buffer_output(context, CL_MEM_WRITE_ONLY | CL_MEM_EXT_PTR_XILINX, WRITEBUF_SIZE,
                                                &outExt, &err));

        OCL_CHECK(err, cl::Buffer num_read_in(context, CL_MEM_WRITE_ONLY, sizeof(num_read_t), NULL, &err));
        OCL_CHECK(err, cl::Buffer num_written_out(context, CL_MEM_WRITE_ONLY, sizeof(num_read_t), NULL, &err));

        cl::Kernel krnl;
        OCL_CHECK(err, krnl = cl::Kernel(program, "process_data", &err));


        num_read_t filein_offset = 0;
        num_read_t fileout_offset = 0;

        while(1) {
	  // nMap P2P device buffers to host access pointers
	  OCL_CHECK(err, void* p2p_in = q.enqueueMapBuffer(buffer_input,      // buffer
							   CL_TRUE,           // blocking call
							   CL_MAP_READ,       // Indicates we will be writing
							   0,                 // buffer offset
							   READBUF_SIZE, // size in bytes
							   nullptr, nullptr,
							   &err)); // error code

            auto numread = pread(fhin.fd(), p2p_in, READBUF_SIZE, filein_offset);
            std::cout << "numread = " << std::hex << numread <<  std::endl;
            if (numread < 0) {
                std::cerr << "ERR: pread failed: "
                            << " error: " << errno << ", " << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            } else if (numread == 0) {
                exit(EXIT_SUCCESS);
            }

	    //OCL_CHECK(err, err = q.enqueueUnmapBuffer(buffer_input, p2p_in));

            OCL_CHECK(err, err = krnl.setArg(0, buffer_input));
            OCL_CHECK(err, err = krnl.setArg(1, static_cast<num_read_t>(numread)));
            OCL_CHECK(err, err = krnl.setArg(2, num_read_in));
            OCL_CHECK(err, err = krnl.setArg(3, buffer_output));
            OCL_CHECK(err, err = krnl.setArg(4, num_written_out));

            // Launch the Kernel
            OCL_CHECK(err, err = q.enqueueTask(krnl));

            // read the counts
            num_read_t numReadIn;
            OCL_CHECK(err, err = q.enqueueReadBuffer(num_read_in, CL_TRUE, 0, sizeof(num_read_t), &numReadIn));

            num_read_t numWrittenOut;
            OCL_CHECK(err, err = q.enqueueReadBuffer(num_written_out, CL_TRUE, 0, sizeof(num_read_t), &numWrittenOut));

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
            filein_offset += numReadIn;
            fileout_offset += WRITEBUF_SIZE;
            std::cout << "Next iteration with read offset (hex) " << std::hex << filein_offset 
                << " and write offset (hex) " << fileout_offset << std::endl;
            /// TO MAKE DEBUGGING SHORTER
            break;
        }
    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

}
