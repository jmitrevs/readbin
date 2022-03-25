// The general header file for the readbin program

#include "detdataformats/wib/WIBFrame.hpp"

//const int READ_SIZE = 0x8000000;
const long READ_SIZE =  0x1d00100;
const long MAX_RECORDS = 64;

constexpr int NUM_CHANNELS = dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_block 
        * dunedaq::detdataformats::wib::WIBFrame::s_num_block_per_frame;

// forward definition to process at most num_to_read events from readbuf;
// actual number read is output in num_read. The variable channels
// represents the output (currently the max value)

void process(uint8_t readbuf[READ_SIZE], int num_to_read, 
             off_t& num_read, uint16_t channels[NUM_CHANNELS]);
