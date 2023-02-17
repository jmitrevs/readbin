CXX=g++
RM=rm -f
CPPFLAGS=-O2 -std=c++17 -I${XILINX_HLS}/include -I../daqdataformats/include -I../detdataformats/include -I${XILINX_XRT}/include -I/cvmfs/sft.cern.ch/lcg/releases/Boost/1.78.0-f6f04/x86_64-centos8-gcc11-opt/include
VPPFLAGS=-I../daqdataformats/include -I../detdataformats/include -I. -I./cnn/firmware
LDFLAGS=-L${XILINX_XRT}/lib -L/cvmfs/sft.cern.ch/lcg/releases/Boost/1.78.0-f6f04/x86_64-centos8-gcc11-opt/lib
LDLIBS=-lboost_program_options -lOpenCL

SRCS=readbin.cpp xcl2.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

KERNEL=process_data
KERNEL_SRCS=process_data.cpp cnn/firmware/vplane.cpp

#PLATFORM=xilinx_u2_gen3x4_xdma_gc_2_202110_1
#PLATFORM=xilinx_u55c_gen3x16_xdma_2_202110_1
PLATFORM=xilinx_u55c_gen3x16_xdma_3_202210_1
#TYPE=sw_emu
#TYPE=hw_emu
TYPE=hw

all: readbin $(KERNEL).xclbin

readbin: $(OBJS)
	$(CXX) $(LDFLAGS) -o readbin $(OBJS) $(LDLIBS)

depend: .depend

$(KERNEL).xclbin: $(KERNEL).xo
	v++ -l -t $(TYPE) --platform $(PLATFORM) $(KERNEL).xo $(VPPFLAGS) -o $(KERNEL).xclbin

$(KERNEL).xo: $(KERNEL_SRCS) readbin.h
	v++ -c -t $(TYPE) --platform $(PLATFORM) -k $(KERNEL) $(VPPFLAGS) $(KERNEL_SRCS) -o $(KERNEL).xo

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS) $(KERNEL).xclbin $(KERNEL).xo

distclean: clean
	$(RM) *~ .depend

setup:
	emconfigutil --platform $(PLATFORM) --nd 1

note:
	export XCL_EMULATION_MODE=$(TYPE)  # doesn't work called here


include .depend
