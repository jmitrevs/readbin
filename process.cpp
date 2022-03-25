//#include <unistd.h>
#include <iostream>
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
             off_t& num_read, uint16_t channels[NUM_CHANNELS]) {

    #pragma HLS ARRAY_PARTITION variable=channels complete

    // initialize
    num_read = 0;
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

    num_read += sizeof(dunedaq::daqdataformats::TriggerRecordHeaderData);

    // this should be followed by component request
    const auto compReq = reinterpret_cast<dunedaq::daqdataformats::ComponentRequest *>(&readbuf[num_read]);

    num_read += sizeof(dunedaq::daqdataformats::ComponentRequest);

    for (int comp = 0; comp < trigRecHeader->num_requested_components; ++comp) {
        if (num_to_read - num_read < sizeof(dunedaq::daqdataformats::FragmentHeader)) {
            std::cerr << "The read buffer is too small to contain the fragment header\n";
            return;
        }            
        const auto fragmentHeader = reinterpret_cast<dunedaq::daqdataformats::FragmentHeader *>(&readbuf[num_read]);
        if (fragmentHeader->fragment_header_marker != dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic) {
            std::cerr << "The parsed fragment header did not have the right magic number: " << std::hex << fragmentHeader->fragment_header_marker
                      << " (expected " << dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic << ")\n";
            return;
        }

        // make sure that all the fragments were read in
        if (num_read + fragmentHeader->size > num_to_read) {
            std::cerr << "The read buffer is too small to contain the fragment\n";
            return;
        }

        num_read += sizeof(dunedaq::daqdataformats::FragmentHeader);

        // let's see the number of frames
        const auto frames_size = fragmentHeader->size - sizeof(dunedaq::daqdataformats::FragmentHeader);
        const int num_frames = frames_size / sizeof(dunedaq::detdataformats::wib::WIBFrame);

        for (int iframe = 0; iframe < num_frames; ++iframe) {
            const auto frame = reinterpret_cast<dunedaq::detdataformats::wib::WIBFrame *>(&readbuf[num_read]);

            // if (iframe == 0) {
            //     outputtext << "Slot = " << frame->get_wib_header()->slot_no 
            //         << ", fiber = " << frame->get_wib_header()->fiber_no << std::endl;
            // }

            for (int iblock = 0; 
                 iblock < dunedaq::detdataformats::wib::WIBFrame::s_num_block_per_frame;
                 ++iblock) {

                const int base_channel = iblock * dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_block;

                for (int ich = 0; 
                     ich < dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_block;
                     ++ich) {
                    # pragma HLS unroll
                    const auto chanval = frame->get_channel(iblock, ich);
                    const auto chan = base_channel + ich; 
                    if (chanval > channels[chan]) {
                        channels[chan] = chanval;
                    }
                }
            }
            num_read += sizeof(dunedaq::detdataformats::wib::WIBFrame);
        }
    }
}
