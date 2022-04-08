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

    static writebuf_t channels_loc[dunedaq::detdataformats::wib::WIBFrame::s_num_ch_per_frame];
    #pragma HLS ARRAY_PARTITION variable=channels_loc complete

    //uint8_t frame_buf[sizeof(dunedaq::detdataformats::wib::WIBFrame)];

    initialize_loop:
    for (int i = 0; i < dunedaq::detdataformats::wib::WIBFrame::s_num_ch_per_frame; ++i) {
        #pragma HLS unroll
        channels_loc[i] = 0;
    }

    num_read_t read_offset = 0;
    num_read_t chan_offset = 0;

    constexpr num_read_t header_size = sizeof(dunedaq::daqdataformats::TriggerRecordHeaderData) + sizeof(dunedaq::daqdataformats::ComponentRequest);
    const num_read_t max_to_start_new = num_to_read - header_size;

    // loop over all the trigger records
    records_loop:
    while(read_offset < max_to_start_new) {

        std::cout << "Current read index: " << std::hex << read_offset << ", max to start = " << max_to_start_new 
                  << ", chan_offset = " << chan_offset << std::endl;

        const auto trigRecHeader = reinterpret_cast<dunedaq::daqdataformats::TriggerRecordHeaderData *>(&readbuf[read_offset]);

        if (trigRecHeader->trigger_record_header_marker != dunedaq::daqdataformats::TriggerRecordHeaderData::s_trigger_record_header_magic) {

            std::cerr << "The parsed trigger record header did not have the right magic number: " << std::hex <<  trigRecHeader->trigger_record_header_marker
                    << " (expected " << dunedaq::daqdataformats::TriggerRecordHeaderData::s_trigger_record_header_magic << ")\n";
            return;
        }

        read_offset += sizeof(dunedaq::daqdataformats::TriggerRecordHeaderData);

        // this should be followed by component request
        const auto compReq = reinterpret_cast<dunedaq::daqdataformats::ComponentRequest *>(&readbuf[read_offset]);

        read_offset += sizeof(dunedaq::daqdataformats::ComponentRequest);

        comp_loop:
        for (int comp = 0; comp < trigRecHeader->num_requested_components; ++comp) {
            if (num_to_read - read_offset < sizeof(dunedaq::daqdataformats::FragmentHeader)) {
                std::cerr << "The read buffer is too small to contain the fragment header\n";
                return;
            }
            const auto fragmentHeader = reinterpret_cast<dunedaq::daqdataformats::FragmentHeader *>(&readbuf[read_offset]);
            if (fragmentHeader->fragment_header_marker != dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic) {
                std::cerr << "The parsed fragment header did not have the right magic number: " << std::hex << fragmentHeader->fragment_header_marker
                        << " (expected " << dunedaq::daqdataformats::FragmentHeader::s_fragment_header_magic << ")\n";
                return;
            }

            // make sure that all the fragments were read in
            if (read_offset + fragmentHeader->size > num_to_read) {
                std::cerr << "The read buffer is too small to contain the fragment\n";
                return;
            }

            read_offset += sizeof(dunedaq::daqdataformats::FragmentHeader);

            // let's see the number of frames
            const auto frames_size = fragmentHeader->size - sizeof(dunedaq::daqdataformats::FragmentHeader);
            const int num_frames = frames_size / sizeof(dunedaq::detdataformats::wib::WIBFrame);

            frame_loop:
            for (int iframe = 0; iframe < num_frames; ++iframe) {

                const auto frame = reinterpret_cast<dunedaq::detdataformats::wib::WIBFrame *>(&readbuf[read_offset]);

                // if (iframe == 0) {
                //     outputtext << "Slot = " << frame->get_wib_header()->slot_no
                //         << ", fiber = " << frame->get_wib_header()->fiber_no << std::endl;
                // }

                block_loop:
                for (int iblock = 0;
                    iblock < dunedaq::detdataformats::wib::WIBFrame::s_num_block_per_frame;
                    ++iblock) {

                    //std::cout << "iframe = " << iframe << ", iblock = " << iblock << ", chan_offset = " << chan_offset << std::endl;
                    const int base_channel_block = iblock * dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_block;

                    adc_loop:
                    for (int iadc = 0;
                        iadc < dunedaq::detdataformats::wib::ColdataBlock::s_num_adc_per_block;
                        ++iadc) {

                        const int base_channel_adc = base_channel_block 
                            + iadc * dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_adc;

                        static writebuf_t chanval[dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_adc];
                        #pragma HLS array_partition variable=chanval complete
                        static writebuf_t curr_best[dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_adc];
                        #pragma HLS array_partition variable=curr_best complete
                        ch_read_loop:
                        for (int ich = 0;
                            ich < dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_adc;
                            ++ich) {
                            #pragma HLS unroll
                            chanval[ich] = frame->get_channel(iblock, iadc, ich);
                            curr_best[ich] = channels_loc[base_channel_adc + ich];
                        }
                        ch_eval_loop:
                        for (int ich = 0;
                            ich < dunedaq::detdataformats::wib::ColdataBlock::s_num_ch_per_adc;
                            ++ich) {
                            # pragma HLS pipeline
                            if (chanval[ich] > curr_best[ich]) {
                                channels_loc[base_channel_adc + ich] = chanval[ich];
                            }
                        }

                    }

                }
                read_offset += sizeof(dunedaq::detdataformats::wib::WIBFrame);
            }
            write_out_loop:
            for (int i = 0; i < dunedaq::detdataformats::wib::WIBFrame::s_num_ch_per_frame; ++i) {
                #pragma HLS unroll
                channels[chan_offset + i] = channels_loc[i];
                channels_loc[i] = 0;
            }
            chan_offset += dunedaq::detdataformats::wib::WIBFrame::s_num_ch_per_frame;
        }
        // Trigger finished, so increment externally-visible count
        *num_read = read_offset;
        *num_written = chan_offset;
    }
}
