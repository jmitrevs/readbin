// The general header file for the readbin program

#include "detdataformats/wib/WIBFrame.hpp"

// Note:  this puts a limit of 4G on the max file size. Can change if needed
using num_read_t = uint32_t;

//const int READ_SIZE = 0x8000000;
constexpr long READ_SIZE =  0xc000'0000;
constexpr long MAX_RECORDS = 0x4000;

using writebuf_t = uint16_t;

constexpr int NUM_CHANNELS = dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_block
        * dunedaq::detdataformats::wib::WIBFrame::s_num_block_per_frame;

// forward definition to process at most num_to_read events from readbuf;
// actual number read is output in num_read. The variable channels
// represents the output (currently the max value)

extern "C" {
void process_data(uint8_t readbuf[READ_SIZE], num_read_t num_to_read,
                  num_read_t* num_read, writebuf_t channels[NUM_CHANNELS], num_read_t *num_written);
}
