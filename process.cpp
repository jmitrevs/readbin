#include <unistd.h>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include "daqdataformats/TriggerRecordHeaderData.hpp"
#include "daqdataformats/FragmentHeader.hpp"
#include "daqdataformats/ComponentRequest.hpp"
#include "detdataformats/wib/WIBFrame.hpp"
#include "readbin.h"

// process a trigger record and subsequent data
void process(uint8_t readbuf[READ_SIZE], uint8_t writebuf[WRITE_SIZE], int numToRead, off_t& numRead, off_t& numWritten) {

    numRead = 0;
    numWritten = 0;

    // The dummy processing now just prints out some info
    std::ostringstream outputtext;

    const auto trigRecHeader = reinterpret_cast<dunedaq::daqdataformats::TriggerRecordHeaderData *>(readbuf);

    if (trigRecHeader->trigger_record_header_marker != dunedaq::daqdataformats::TriggerRecordHeaderData::s_trigger_record_header_magic) {
        std::ostringstream error_msg;
        error_msg << "The parsed trigger record header did not have the right magic number: " << std::hex <<  trigRecHeader->trigger_record_header_marker
                  << " (expected " << dunedaq::daqdataformats::TriggerRecordHeaderData::s_trigger_record_header_magic << ")";
        throw std::runtime_error(error_msg.str());
    }

    numRead += sizeof(dunedaq::daqdataformats::TriggerRecordHeaderData);
    outputtext << "Trigger record:\t" << *trigRecHeader << std::endl;

    // this should be followed by component request
    const auto compReq = reinterpret_cast<dunedaq::daqdataformats::ComponentRequest *>(&readbuf[numRead]);

    numRead += sizeof(dunedaq::daqdataformats::ComponentRequest);
    outputtext << "Requst:\t" << *compReq << std::endl;

    for (int comp = 0; comp < trigRecHeader->num_requested_components; ++comp) {
        if (READ_SIZE - numRead < sizeof(dunedaq::daqdataformats::FragmentHeader)) {
            throw std::runtime_error("The read buffer is too small to contain the fragment header");
        }            
        const auto fragmentHeader = reinterpret_cast<dunedaq::daqdataformats::FragmentHeader *>(&readbuf[numRead]);
        if (fragmentHeader->fragment_header_marker != dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic) {
            std::ostringstream error_msg;
            error_msg << "The parsed fragment header did not have the right magic number: " << std::hex << fragmentHeader->fragment_header_marker
                      << " (expected " << dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic << ")";
            throw std::runtime_error(error_msg.str());
        }

        // make sure that all the fragments were read in
        if (numRead + fragmentHeader->size > READ_SIZE) {
            throw std::runtime_error("The read buffer is too small to contain the fragment");
        }

        numRead += sizeof(dunedaq::daqdataformats::FragmentHeader);
        outputtext << "Fragment:\t" << *fragmentHeader << std::endl;

        // let's see the number of frames
        const auto framesSize = fragmentHeader->size - sizeof(dunedaq::daqdataformats::FragmentHeader);
        const int numFrames = framesSize / sizeof(dunedaq::detdataformats::wib::WIBFrame);

        for (int iframe = 0; iframe < numFrames; ++iframe) {
            const auto frame = reinterpret_cast<dunedaq::detdataformats::wib::WIBFrame *>(&readbuf[numRead]);

            // to control the output size, only print the first 10
            if (iframe < 10) {
                outputtext << *frame;
            }
            numRead += sizeof(dunedaq::detdataformats::wib::WIBFrame);
        }
        
    }

    auto outputstr = outputtext.str();

    numWritten = outputstr.size();

    if (numWritten > WRITE_SIZE) {
        std::cerr << "numWritten = " << std::hex << numWritten << " is too large" << std::endl;
    }
    for (int i = 0; i < outputstr.size(); i++) {
        writebuf[i] = outputstr[i];
    }

}
