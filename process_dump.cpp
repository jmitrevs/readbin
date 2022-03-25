#include <unistd.h>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "daqdataformats/TriggerRecordHeaderData.hpp"
#include "daqdataformats/FragmentHeader.hpp"
#include "daqdataformats/ComponentRequest.hpp"
#include "detdataformats/wib/WIBFrame.hpp"
#include "readbin.h"

// process a trigger record and subsequent data
void process(uint8_t readbuf[READ_SIZE], uint8_t writebuf[WRITE_SIZE], int numToRead, off_t& num_read, off_t& num_written) {

    num_read = 0;
    num_written = 0;

    // The dummy processing now just prints out some info
    std::ostringstream outputtext;

    const auto trigRecHeader = reinterpret_cast<dunedaq::daqdataformats::TriggerRecordHeaderData *>(readbuf);

    if (trigRecHeader->trigger_record_header_marker != dunedaq::daqdataformats::TriggerRecordHeaderData::s_trigger_record_header_magic) {
        std::ostringstream error_msg;
        error_msg << "The parsed trigger record header did not have the right magic number: " << std::hex <<  trigRecHeader->trigger_record_header_marker
                  << " (expected " << dunedaq::daqdataformats::TriggerRecordHeaderData::s_trigger_record_header_magic << ")";
        throw std::runtime_error(error_msg.str());
    }

    num_read += sizeof(dunedaq::daqdataformats::TriggerRecordHeaderData);
    outputtext << "Trigger record:\t" << *trigRecHeader << std::endl;

    // this should be followed by component request
    const auto compReq = reinterpret_cast<dunedaq::daqdataformats::ComponentRequest *>(&readbuf[num_read]);

    num_read += sizeof(dunedaq::daqdataformats::ComponentRequest);
    outputtext << "Requst:\t" << *compReq << std::endl;

    for (int comp = 0; comp < trigRecHeader->num_requested_components; ++comp) {
        if (READ_SIZE - num_read < sizeof(dunedaq::daqdataformats::FragmentHeader)) {
            throw std::runtime_error("The read buffer is too small to contain the fragment header");
        }            
        const auto fragmentHeader = reinterpret_cast<dunedaq::daqdataformats::FragmentHeader *>(&readbuf[num_read]);
        if (fragmentHeader->fragment_header_marker != dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic) {
            std::ostringstream error_msg;
            error_msg << "The parsed fragment header did not have the right magic number: " << std::hex << fragmentHeader->fragment_header_marker
                      << " (expected " << dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic << ")";
            throw std::runtime_error(error_msg.str());
        }

        // make sure that all the fragments were read in
        if (num_read + fragmentHeader->size > READ_SIZE) {
            throw std::runtime_error("The read buffer is too small to contain the fragment");
        }

        num_read += sizeof(dunedaq::daqdataformats::FragmentHeader);
        outputtext << "Fragment:\t" << *fragmentHeader << std::endl;

        // let's see the number of frames
        const auto frames_size = fragmentHeader->size - sizeof(dunedaq::daqdataformats::FragmentHeader);
        const int num_frames = frames_size / sizeof(dunedaq::detdataformats::wib::WIBFrame);


        constexpr int num_channels = dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_block 
            * dunedaq::detdataformats::wib::WIBFrame::s_num_block_per_frame;

        outputtext << "num_frames = " << num_frames << ", num channels per block = " 
            << dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_block 
            << ", num_channels = " << num_channels << std::endl;

        std::vector<uint16_t> channels[num_channels];

        for (int iframe = 0; iframe < num_frames; ++iframe) {
            const auto frame = reinterpret_cast<dunedaq::detdataformats::wib::WIBFrame *>(&readbuf[num_read]);

            if (iframe == 0) {
                outputtext << "Slot = " << frame->get_wib_header()->slot_no 
                    << ", fiber = " << frame->get_wib_header()->fiber_no << std::endl;
            }

            for (int ich = 0; ich < num_channels; ++ich) {
                channels[ich].push_back(frame->get_channel(ich));
            }
            num_read += sizeof(dunedaq::detdataformats::wib::WIBFrame);
        }
        for (int ich = 0; ich < num_channels; ++ich) {
            std::cout << "channels[" << ich << "] has size " <<  channels[ich].size() << std::endl;
        }
    }

    auto outputstr = outputtext.str();

    num_written = outputstr.size();

    if (num_written > WRITE_SIZE) {
        std::cerr << "num_written = " << std::hex << num_written << " is too large" << std::endl;
    }
    for (int i = 0; i < outputstr.size(); i++) {
        writebuf[i] = outputstr[i];
    }

}
