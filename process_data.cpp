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


void process_data(uint8_t readbuf[READ_SIZE], num_read_t num_to_read,
                  num_read_t* num_read, writebuf_t channels[NUM_CHANNELS], num_read_t *num_written) {

    // initialize
    *num_read = 0;
    *num_written = 0;

    initialize_loop:
    for (int ich = 0; ich < NUM_CHANNELS; ++ich) {
        #pragma HLS unroll
        channels[ich] = 0;
    }

    num_read_t read_index = 0;
    num_read_t chan_index = 0;

    constexpr num_read_t header_size = sizeof(dunedaq::daqdataformats::TriggerRecordHeaderData) + sizeof(dunedaq::daqdataformats::ComponentRequest);
    const num_read_t max_to_start_new = num_to_read - header_size;

    // loop over all the trigger records
    records_loop:
    while(read_index < max_to_start_new) {

        std::cout << "Current read index: " << std::hex << read_index << ", max to start = " << max_to_start_new << std::endl;

        const auto trigRecHeader = reinterpret_cast<dunedaq::daqdataformats::TriggerRecordHeaderData *>(&readbuf[read_index]);

        if (trigRecHeader->trigger_record_header_marker != dunedaq::daqdataformats::TriggerRecordHeaderData::s_trigger_record_header_magic) {

            std::cerr << "The parsed trigger record header did not have the right magic number: " << std::hex <<  trigRecHeader->trigger_record_header_marker
                    << " (expected " << dunedaq::daqdataformats::TriggerRecordHeaderData::s_trigger_record_header_magic << ")\n";
            return;
        }

        read_index += sizeof(dunedaq::daqdataformats::TriggerRecordHeaderData);

        // this should be followed by component request
        const auto compReq = reinterpret_cast<dunedaq::daqdataformats::ComponentRequest *>(&readbuf[read_index]);

        read_index += sizeof(dunedaq::daqdataformats::ComponentRequest);

        comp_loop:
        for (int comp = 0; comp < trigRecHeader->num_requested_components; ++comp) {
            if (num_to_read - read_index < sizeof(dunedaq::daqdataformats::FragmentHeader)) {
                std::cerr << "The read buffer is too small to contain the fragment header\n";
                return;
            }
            const auto fragmentHeader = reinterpret_cast<dunedaq::daqdataformats::FragmentHeader *>(&readbuf[read_index]);
            if (fragmentHeader->fragment_header_marker != dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic) {
                std::cerr << "The parsed fragment header did not have the right magic number: " << std::hex << fragmentHeader->fragment_header_marker
                        << " (expected " << dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic << ")\n";
                return;
            }

            // make sure that all the fragments were read in
            if (read_index + fragmentHeader->size > num_to_read) {
                std::cerr << "The read buffer is too small to contain the fragment\n";
                return;
            }

            read_index += sizeof(dunedaq::daqdataformats::FragmentHeader);

            // let's see the number of frames
            const auto frames_size = fragmentHeader->size - sizeof(dunedaq::daqdataformats::FragmentHeader);
            const int num_frames = frames_size / sizeof(dunedaq::detdataformats::wib::WIBFrame);

            frame_loop:
            for (int iframe = 0; iframe < num_frames; ++iframe) {
                const auto frame = reinterpret_cast<dunedaq::detdataformats::wib::WIBFrame *>(&readbuf[read_index]);

                // if (iframe == 0) {
                //     outputtext << "Slot = " << frame->get_wib_header()->slot_no
                //         << ", fiber = " << frame->get_wib_header()->fiber_no << std::endl;
                // }

                block_loop:
                for (int iblock = 0;
                    iblock < dunedaq::detdataformats::wib::WIBFrame::s_num_block_per_frame;
                    ++iblock) {

                    adc_loop:
                    for (int iadc = 0;
                        iadc < dunedaq::detdataformats::wib::ColdataBlock::s_num_adc_per_block;
                        ++iadc) {

                        writebuf_t chanval[dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_adc];
                        #pragma HLS array_partition variable=chanval complete
                        writebuf_t curr_best[dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_adc];
                        #pragma HLS array_partition variable=curr_best complete
                        ch_read_loop:
                        for (int ich = 0;
                            ich < dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_adc;
                            ++ich) {
                            #pragma HLS unroll
                            chanval[ich] = frame->get_channel(iblock, iadc, ich);
                            curr_best[ich] = channels[chan_index + ich];
                        }
                        ch_eval_loop:
                        for (int ich = 0;
                            ich < dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_adc;
                            ++ich) {
                            # pragma HLS pipeline
                            if (chanval[ich] > curr_best[ich]) {
                                channels[chan_index] = chanval[ich];
                            }
                            ++chan_index;
                        }

                    }

                }
                read_index += sizeof(dunedaq::detdataformats::wib::WIBFrame);
            }
        }
        // Trigger finished, so increment externally-visible count
        *num_read = read_index;
        *num_written = chan_index;
    }

}
