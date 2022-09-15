#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include "ap_int.h"
#include "ap_fixed.h"

#include "nnet_utils/nnet_helpers.h"
//hls-fpga-machine-learning insert includes
#include "nnet_utils/nnet_activation.h"
#include "nnet_utils/nnet_activation_stream.h"
#include "nnet_utils/nnet_conv1d.h"
#include "nnet_utils/nnet_conv1d_stream.h"
#include "nnet_utils/nnet_dense.h"
#include "nnet_utils/nnet_dense_compressed.h"
#include "nnet_utils/nnet_dense_stream.h"
#include "nnet_utils/nnet_pooling.h"
#include "nnet_utils/nnet_pooling_stream.h"
 
//hls-fpga-machine-learning insert weights
#include "weights/w2.h"
#include "weights/b2.h"
#include "weights/w5.h"
#include "weights/b5.h"
#include "weights/w8.h"
#include "weights/b8.h"
#include "weights/w12.h"
#include "weights/b12.h"

//hls-fpga-machine-learning insert layer-config
// Conv_1
struct config2_mult : nnet::dense_config {
    static const unsigned n_in = 3;
    static const unsigned n_out = 16;
    static const unsigned reuse_factor = 48;
    static const unsigned strategy = nnet::resource;
    static const unsigned n_zeros = 0;
    static const unsigned multiplier_limit = DIV_ROUNDUP(n_in * n_out, reuse_factor) - n_zeros / reuse_factor;
    typedef model_default_t accum_t;
    typedef bias2_t bias_t;
    typedef weight2_t weight_t;
    template<class x_T, class y_T>
    using product = nnet::product::mult<x_T, y_T>;
};

struct config2 : nnet::conv1d_config {
    static const unsigned pad_left = 0;
    static const unsigned pad_right = 0;
    static const unsigned in_width = 200;
    static const unsigned n_chan = 1;
    static const unsigned filt_width = 3;
    static const unsigned kernel_size = filt_width;
    static const unsigned n_filt = 16;
    static const unsigned stride_width = 2;
    static const unsigned dilation = 1;
    static const unsigned out_width = 99;
    static const unsigned reuse_factor = 48;
    static const unsigned n_zeros = 3;
    static const bool store_weights_in_bram = false;
    static const unsigned strategy = nnet::resource;
    static const nnet::conv_implementation implementation = nnet::conv_implementation::linebuffer;
    static const unsigned min_width = 6;
    static const ap_uint<filt_width> pixels[min_width];
    typedef model_default_t accum_t;
    typedef bias2_t bias_t;
    typedef weight2_t weight_t;
    typedef config2_mult mult_config;
};
const ap_uint<config2::filt_width> config2::pixels[] = {1,2,5,2,4,0};

// Conv_1_quantized_relu
struct relu_config3 : nnet::activ_config {
    static const unsigned n_in = 1584;
    static const unsigned table_size = 1024;
    static const unsigned io_type = nnet::io_stream;
    static const unsigned reuse_factor = 128;
    typedef Conv_1_quantized_relu_table_t table_t;
};

// MaxPool_1
struct config4 : nnet::pooling1d_config {
    static const unsigned n_in = 99;
    static const unsigned n_out = 49;
    static const unsigned n_filt = 16;
    static const unsigned pool_width = 2;

    static const unsigned filt_width = pool_width;
    static const unsigned n_chan = n_filt;

    static const unsigned pad_left = 0;
    static const unsigned pad_right = 0;
    static const unsigned stride_width = 2;
    static const nnet::Pool_Op pool_op = nnet::Max;
    static const nnet::conv_implementation implementation = nnet::conv_implementation::linebuffer;
    static const unsigned reuse_factor = 128;
    typedef maxpool_1_default_t accum_t;
};

// Conv_2
struct config5_mult : nnet::dense_config {
    static const unsigned n_in = 80;
    static const unsigned n_out = 32;
    static const unsigned reuse_factor = 160;
    static const unsigned strategy = nnet::resource;
    static const unsigned n_zeros = 0;
    static const unsigned multiplier_limit = DIV_ROUNDUP(n_in * n_out, reuse_factor) - n_zeros / reuse_factor;
    typedef model_default_t accum_t;
    typedef bias5_t bias_t;
    typedef weight5_t weight_t;
    template<class x_T, class y_T>
    using product = nnet::product::mult<x_T, y_T>;
};

struct config5 : nnet::conv1d_config {
    static const unsigned pad_left = 0;
    static const unsigned pad_right = 0;
    static const unsigned in_width = 49;
    static const unsigned n_chan = 16;
    static const unsigned filt_width = 5;
    static const unsigned kernel_size = filt_width;
    static const unsigned n_filt = 32;
    static const unsigned stride_width = 2;
    static const unsigned dilation = 1;
    static const unsigned out_width = 23;
    static const unsigned reuse_factor = 160;
    static const unsigned n_zeros = 1388;
    static const bool store_weights_in_bram = false;
    static const unsigned strategy = nnet::resource;
    static const nnet::conv_implementation implementation = nnet::conv_implementation::linebuffer;
    static const unsigned min_width = 9;
    static const ap_uint<filt_width> pixels[min_width];
    typedef model_default_t accum_t;
    typedef bias5_t bias_t;
    typedef weight5_t weight_t;
    typedef config5_mult mult_config;
};
const ap_uint<config5::filt_width> config5::pixels[] = {1,2,5,10,21,10,20,8,16};

// Conv_2_quantized_relu
struct relu_config6 : nnet::activ_config {
    static const unsigned n_in = 736;
    static const unsigned table_size = 1024;
    static const unsigned io_type = nnet::io_stream;
    static const unsigned reuse_factor = 128;
    typedef Conv_2_quantized_relu_table_t table_t;
};

// MaxPool_2
struct config7 : nnet::pooling1d_config {
    static const unsigned n_in = 23;
    static const unsigned n_out = 11;
    static const unsigned n_filt = 32;
    static const unsigned pool_width = 2;

    static const unsigned filt_width = pool_width;
    static const unsigned n_chan = n_filt;

    static const unsigned pad_left = 0;
    static const unsigned pad_right = 0;
    static const unsigned stride_width = 2;
    static const nnet::Pool_Op pool_op = nnet::Max;
    static const nnet::conv_implementation implementation = nnet::conv_implementation::linebuffer;
    static const unsigned reuse_factor = 128;
    typedef maxpool_2_default_t accum_t;
};

// Conv_3
struct config8_mult : nnet::dense_config {
    static const unsigned n_in = 288;
    static const unsigned n_out = 64;
    static const unsigned reuse_factor = 144;
    static const unsigned strategy = nnet::resource;
    static const unsigned n_zeros = 0;
    static const unsigned multiplier_limit = DIV_ROUNDUP(n_in * n_out, reuse_factor) - n_zeros / reuse_factor;
    typedef model_default_t accum_t;
    typedef bias8_t bias_t;
    typedef weight8_t weight_t;
    template<class x_T, class y_T>
    using product = nnet::product::mult<x_T, y_T>;
};

struct config8 : nnet::conv1d_config {
    static const unsigned pad_left = 0;
    static const unsigned pad_right = 0;
    static const unsigned in_width = 11;
    static const unsigned n_chan = 32;
    static const unsigned filt_width = 9;
    static const unsigned kernel_size = filt_width;
    static const unsigned n_filt = 64;
    static const unsigned stride_width = 1;
    static const unsigned dilation = 1;
    static const unsigned out_width = 3;
    static const unsigned reuse_factor = 144;
    static const unsigned n_zeros = 13172;
    static const bool store_weights_in_bram = false;
    static const unsigned strategy = nnet::resource;
    static const nnet::conv_implementation implementation = nnet::conv_implementation::linebuffer;
    static const unsigned min_width = 11;
    static const ap_uint<filt_width> pixels[min_width];
    typedef model_default_t accum_t;
    typedef bias8_t bias_t;
    typedef weight8_t weight_t;
    typedef config8_mult mult_config;
};
const ap_uint<config8::filt_width> config8::pixels[] = {1,3,7,14,28,56,112,224,448,384,256};

// Conv_3_quantized_relu
struct relu_config9 : nnet::activ_config {
    static const unsigned n_in = 192;
    static const unsigned table_size = 1024;
    static const unsigned io_type = nnet::io_stream;
    static const unsigned reuse_factor = 128;
    typedef Conv_3_quantized_relu_table_t table_t;
};

// GlobalMaxPool_1
struct config10 : nnet::pooling1d_config {
    static const unsigned n_in = 3;
    static const unsigned n_filt = 64;
    static const nnet::Pool_Op pool_op = nnet::Max;
    static const unsigned reuse_factor = 128;
    typedef globalmaxpool_1_default_t accum_t;
};

// wavrec_out
struct config12 : nnet::dense_config {
    static const unsigned n_in = 64;
    static const unsigned n_out = 1;
    static const unsigned io_type = nnet::io_stream;
    static const unsigned strategy = nnet::resource;
    static const unsigned reuse_factor = 64;
    static const unsigned n_zeros = 21;
    static const unsigned n_nonzeros = 43;
    static const unsigned multiplier_limit = DIV_ROUNDUP(n_in * n_out, reuse_factor) - n_zeros / reuse_factor;
    static const bool store_weights_in_bram = false;
    typedef model_default_t accum_t;
    typedef bias12_t bias_t;
    typedef weight12_t weight_t;
    typedef layer12_index index_t;
    template<class x_T, class y_T>
    using product = nnet::product::mult<x_T, y_T>;
};

// wavrec_out_quantized_sigmoid
struct hard_sigmoid_config13 {
    static const unsigned n_in = 1;
    static const slope13_t slope;
    static const shift13_t shift;
    static const unsigned io_type = nnet::io_stream;
    static const unsigned reuse_factor = 128;
};
const slope13_t hard_sigmoid_config13::slope = 0.5;
const shift13_t hard_sigmoid_config13::shift = 0.5;


#endif
