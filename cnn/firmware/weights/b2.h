//Numpy array shape [16]
//Min -0.250000000000
//Max 0.250000000000
//Number of zeros 8

#ifndef B2_H_
#define B2_H_

#ifndef __SYNTHESIS__
bias2_t b2[16];
#else
bias2_t b2[16] = {0.25, 0.00, -0.25, -0.25, -0.25, -0.25, 0.00, -0.25, -0.25, -0.25, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00};
#endif

#endif
