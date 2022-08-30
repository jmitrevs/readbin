//#include <unistd.h>
#include <iostream>
#include <cmath>
//#include <cstdint>
//#include <sstream>
//#include <stdexcept>
//#include <vector>
#include "daqdataformats/TriggerRecordHeaderData.hpp"
#include "daqdataformats/FragmentHeader.hpp"
#include "daqdataformats/ComponentRequest.hpp"
#include "detdataformats/wib/WIBFrame.hpp"
#include "readbin.h"

// process a trigger record and subsequent data


void process_data(uint8_t readbuf[READ_SIZE], int num_to_read,
                  off_t* num_read, uint16_t channels[NUM_CHANNELS]) {

    // initialize
    *num_read = 0;

    initialize_loop:
    for (int ich = 0; ich < NUM_CHANNELS; ++ich) {
        #pragma HLS unroll
        channels[ich] = 0;
    }

    const auto trigRecHeader = reinterpret_cast<dunedaq::daqdataformats::TriggerRecordHeaderData *>(readbuf);

    if (trigRecHeader->trigger_record_header_marker != dunedaq::daqdataformats::TriggerRecordHeaderData::s_trigger_record_header_magic) {

        std::cerr << "The parsed trigger record header did not have the right magic number: " << std::hex <<  trigRecHeader->trigger_record_header_marker
                  << " (expected " << dunedaq::daqdataformats::TriggerRecordHeaderData::s_trigger_record_header_magic << ")\n";
        return;
    }

    *num_read += sizeof(dunedaq::daqdataformats::TriggerRecordHeaderData);

    //std::cout << "trigRecHeader->num_requested_components = " << std::dec << trigRecHeader->num_requested_components << std::endl;


    comp_loop:
    for (int comp = 0; comp < trigRecHeader->num_requested_components; ++comp) {
        // this should be followed by component request
        // const auto compReq = reinterpret_cast<dunedaq::daqdataformats::ComponentRequest *>(&readbuf[*num_read]);

        *num_read += sizeof(dunedaq::daqdataformats::ComponentRequest);

        if (num_to_read - *num_read < sizeof(dunedaq::daqdataformats::FragmentHeader)) {
            std::cerr << "The read buffer is too small to contain the fragment header\n";
            return;
        }
        const auto fragmentHeader = reinterpret_cast<dunedaq::daqdataformats::FragmentHeader *>(&readbuf[*num_read]);
        if (fragmentHeader->fragment_header_marker != dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic) {
            std::cerr << "The parsed fragment header did not have the right magic number: " << std::hex << fragmentHeader->fragment_header_marker
                     << " (expected " << dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic << ")\n";
            return;
        }

        // make sure that all the fragments were read in
        if (*num_read + fragmentHeader->size > num_to_read) {
            std::cerr << "The read buffer is too small to contain the fragment\n";
            return;
        }

        *num_read += sizeof(dunedaq::daqdataformats::FragmentHeader);

        // let's see the number of frames
        const auto frames_size = fragmentHeader->size - sizeof(dunedaq::daqdataformats::FragmentHeader);
        const int num_frames = frames_size / sizeof(dunedaq::detdataformats::wib::WIBFrame);

        std::cout << "num_frames (hex) = " << std::hex << num_frames
            << ", num blocks = " << dunedaq::detdataformats::wib::WIBFrame::s_num_block_per_frame
            << ", num adc = " << dunedaq::detdataformats::wib::ColdataBlock::s_num_adc_per_block
            << ", num chan = " << dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_adc << std::dec << std::endl;


        static double sum[dunedaq::detdataformats::wib::WIBFrame::s_num_ch_per_frame];
        static double sum2[dunedaq::detdataformats::wib::WIBFrame::s_num_ch_per_frame];

        for (int i = 0; i < dunedaq::detdataformats::wib::WIBFrame::s_num_ch_per_frame; i++) {
            sum[i] = 0;
            sum2[i] = 0;
        }

        //const uint8_t ch = 1;

        //std::cout << "Printing first 50 frames for channel " << static_cast<int>(ch) << std::endl;

        uint32_t slot = 0, fiber = 0;
        frame_loop:
        for (int iframe = 0; iframe < num_frames; ++iframe) {
            const auto frame = reinterpret_cast<dunedaq::detdataformats::wib::WIBFrame *>(&readbuf[*num_read]);


            if (iframe == 0) {
                slot = frame->get_wib_header()->slot_no;
                fiber = frame->get_wib_header()->fiber_no;
                std::cout << "slot, fiber: "  << slot << ", " << fiber << std::endl;
            }

            for (unsigned ch = 0;
                ch < dunedaq::detdataformats::wib::WIBFrame::s_num_ch_per_frame;
                ch++) {
                auto val = static_cast<double>(frame->get_channel(ch));
                sum[ch] += val;
                sum2[ch] += (val * val);
            }
            *num_read += sizeof(dunedaq::detdataformats::wib::WIBFrame);
        }

        int count = 0;
        double ave_ave = 0.0;
        double ave_stddev = 0.0;
        for (unsigned ch = 0;
            ch < dunedaq::detdataformats::wib::WIBFrame::s_num_ch_per_frame;
            ch++) {
            auto ave = sum[ch]/ num_frames;
            auto ave2 = sum2[ch] / num_frames;
            auto var = ave2 - ave*ave;
            auto stddev = std::sqrt(var);
            //std::cout << "chan " << ch << ": average = " << ave << ", var = " << var << ", stddev = " << stddev << std::endl;
            if (ave > 2000) {
                std::cout << ch << "," << std::endl;
                count++;
                ave_ave += ave;
                ave_stddev += stddev;
            } else {
                //std::cout << "false," << std::endl;
            }

        }
        std::cout << "overall average = " << ave_ave/count << ", stddev = " << ave_stddev/count << std::endl;
        std::cout << std::endl;
    }
}
