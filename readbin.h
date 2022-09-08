// The general header file for the readbin program
#ifndef READBIN_READBIN_H_
#define READBIN_READBIN_H_

#include "detdataformats/wib/WIBFrame.hpp"
#include "cnn/firmware/defines.h"
#include "mask.h"

// Note:  this puts a limit of 4G on the max file size. Can change if needed
using num_read_t = uint32_t;

//constexpr long READ_SIZE =   0x800'0000;
constexpr long READ_SIZE =  0x200'0000;
constexpr long MAX_RECORDS = 512;

using writebuf_t = result_t::value_type;

constexpr int NUM_CHANNELS = dunedaq::detdataformats::wib::WIBFrame::s_num_ch_per_frame * MAX_RECORDS;

// forward definition to process at most num_to_read events from readbuf;
// actual number read is output in num_read. The variable channels
// represents the output (currently the max value)

void process_data(uint8_t readbuf[READ_SIZE], num_read_t num_to_read, num_read_t initial_offset,
                  num_read_t* num_read, writebuf_t channels[NUM_CHANNELS], num_read_t *num_written);

#endif // READBIN_READBIN_H_
