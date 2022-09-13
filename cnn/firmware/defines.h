#ifndef DEFINES_H_
#define DEFINES_H_

#include "ap_int.h"
#include "ap_fixed.h"
#include "nnet_utils/nnet_types.h"
#include <cstddef>
#include <cstdio>

//hls-fpga-machine-learning insert numbers
#define N_INPUT_1_1 200
#define N_INPUT_2_1 1
#define N_OUTPUTS_2 99
#define N_FILT_2 16
#define N_OUTPUTS_4 49
#define N_FILT_4 16
#define N_OUTPUTS_5 23
#define N_FILT_5 32
#define N_OUTPUTS_7 11
#define N_FILT_7 32
#define N_OUTPUTS_8 3
#define N_FILT_8 64
#define N_FILT_10 64
#define N_SIZE_1_11 64
#define N_LAYER_12 1

//hls-fpga-machine-learning insert layer-precision
typedef ap_fixed<16,6> model_default_t;
typedef nnet::array<ap_fixed<16,6>, 1*1> input_t;
typedef nnet::array<ap_fixed<16,6>, 16*1> layer2_t;
typedef ap_fixed<4,2> weight2_t;
typedef ap_fixed<4,2> bias2_t;
typedef nnet::array<ap_ufixed<5,1,AP_TRN,AP_WRAP>, 16*1> layer3_t;
typedef ap_fixed<18,8> Conv_1_quantized_relu_table_t;
typedef ap_fixed<16,6> maxpool_1_default_t;
typedef nnet::array<ap_fixed<16,6>, 16*1> layer4_t;
typedef nnet::array<ap_fixed<16,6>, 32*1> layer5_t;
typedef ap_fixed<4,2> weight5_t;
typedef ap_fixed<4,2> bias5_t;
typedef nnet::array<ap_ufixed<5,1,AP_TRN,AP_WRAP>, 32*1> layer6_t;
typedef ap_fixed<18,8> Conv_2_quantized_relu_table_t;
typedef ap_fixed<16,6> maxpool_2_default_t;
typedef nnet::array<ap_fixed<16,6>, 32*1> layer7_t;
typedef nnet::array<ap_fixed<16,6>, 64*1> layer8_t;
typedef ap_fixed<4,2> weight8_t;
typedef ap_fixed<4,2> bias8_t;
typedef nnet::array<ap_ufixed<5,1,AP_TRN,AP_WRAP>, 64*1> layer9_t;
typedef ap_fixed<18,8> Conv_3_quantized_relu_table_t;
typedef ap_fixed<16,6> globalmaxpool_1_default_t;
typedef nnet::array<ap_fixed<16,6>, 64*1> layer10_t;
typedef nnet::array<ap_fixed<16,6>, 1*1> layer12_t;
typedef ap_fixed<4,2> weight12_t;
typedef ap_fixed<4,2> bias12_t;
typedef ap_uint<1> layer12_index;
typedef nnet::array<ap_ufixed<5,0,AP_TRN,AP_WRAP>, 1*1> result_t;
typedef ap_ufixed<1,0> slope13_t;
typedef ap_ufixed<1,0> shift13_t;
typedef ap_fixed<18,8> wavrec_out_quantized_sigmoid_table_t;

#endif
