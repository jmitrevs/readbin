//
//    rfnoc-hls-neuralnet: Vivado HLS code for neural-net building blocks
//
//    Copyright (C) 2017 EJ Kreinar
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//#include <iostream>

#include "vplane.h"
#include "parameters.h"

void vplane(
    hls::stream<input_t> &Conv_1_input,
    hls::stream<result_t> &layer13_out
) {

    //hls-fpga-machine-learning insert IO
    //#pragma HLS INTERFACE axis port=Conv_1_input,layer13_out
    #pragma HLS DATAFLOW 

	//hls-fpga-machine-learning insert weights
	#include "weights/w2.h"
	#include "weights/b2.h"
	#include "weights/w5.h"
	#include "weights/b5.h"
	#include "weights/w8.h"
	#include "weights/b8.h"
	#include "weights/w12.h"
	#include "weights/b12.h"

#ifndef __SYNTHESIS__
    static bool loaded_weights = false;
    if (!loaded_weights) {
        //hls-fpga-machine-learning insert load weights
        nnet::load_weights_from_txt<weight2_t, 48>(w2, "w2.txt");
        nnet::load_weights_from_txt<bias2_t, 16>(b2, "b2.txt");
        nnet::load_weights_from_txt<weight5_t, 2560>(w5, "w5.txt");
        nnet::load_weights_from_txt<bias5_t, 32>(b5, "b5.txt");
        nnet::load_weights_from_txt<weight8_t, 18432>(w8, "w8.txt");
        nnet::load_weights_from_txt<bias8_t, 64>(b8, "b8.txt");
        nnet::load_weights_from_txt<weight12_t, 64>(w12, "w12.txt");
        nnet::load_weights_from_txt<bias12_t, 1>(b12, "b12.txt");
        loaded_weights = true;
    }
#endif

    // ****************************************
    // NETWORK INSTANTIATION
    // ****************************************

    //hls-fpga-machine-learning insert layers

    hls::stream<layer2_t> layer2_out("layer2_out");
    #pragma HLS STREAM variable=layer2_out depth=99
    nnet::conv_1d_cl<input_t, layer2_t, config2>(Conv_1_input, layer2_out, w2, b2); // Conv_1

    hls::stream<layer3_t> layer3_out("layer3_out");
    #pragma HLS STREAM variable=layer3_out depth=99
    nnet::relu<layer2_t, layer3_t, relu_config3>(layer2_out, layer3_out); // Conv_1_quantized_relu

    hls::stream<layer4_t> layer4_out("layer4_out");
    #pragma HLS STREAM variable=layer4_out depth=49
    nnet::pooling1d_cl<layer3_t, layer4_t, config4>(layer3_out, layer4_out); // MaxPool_1

    hls::stream<layer5_t> layer5_out("layer5_out");
    #pragma HLS STREAM variable=layer5_out depth=23
    nnet::conv_1d_cl<layer4_t, layer5_t, config5>(layer4_out, layer5_out, w5, b5); // Conv_2

    hls::stream<layer6_t> layer6_out("layer6_out");
    #pragma HLS STREAM variable=layer6_out depth=23
    nnet::relu<layer5_t, layer6_t, relu_config6>(layer5_out, layer6_out); // Conv_2_quantized_relu

    hls::stream<layer7_t> layer7_out("layer7_out");
    #pragma HLS STREAM variable=layer7_out depth=11
    nnet::pooling1d_cl<layer6_t, layer7_t, config7>(layer6_out, layer7_out); // MaxPool_2

    hls::stream<layer8_t> layer8_out("layer8_out");
    #pragma HLS STREAM variable=layer8_out depth=3
    nnet::conv_1d_cl<layer7_t, layer8_t, config8>(layer7_out, layer8_out, w8, b8); // Conv_3

    hls::stream<layer9_t> layer9_out("layer9_out");
    #pragma HLS STREAM variable=layer9_out depth=3
    nnet::relu<layer8_t, layer9_t, relu_config9>(layer8_out, layer9_out); // Conv_3_quantized_relu

    hls::stream<layer10_t> layer10_out("layer10_out");
    #pragma HLS STREAM variable=layer10_out depth=1
    nnet::global_pooling1d_cl<layer9_t, layer10_t, config10>(layer9_out, layer10_out); // GlobalMaxPool_1

    hls::stream<layer12_t> layer12_out("layer12_out");
    #pragma HLS STREAM variable=layer12_out depth=1
    nnet::dense<layer10_t, layer12_t, config12>(layer10_out, layer12_out, w12, b12); // wavrec_out

    nnet::hard_sigmoid<layer12_t, result_t, hard_sigmoid_config13>(layer12_out, layer13_out); // wavrec_out_quantized_sigmoid

}
