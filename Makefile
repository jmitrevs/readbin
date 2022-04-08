CXX=g++
RM=rm -f
CPPFLAGS=-g -std=c++17 -I../daqdataformats/include -I../detdataformats/include -I${XILINX_XRT}/include
VPPFLAGS=--save-temps -I../daqdataformats/include -I../detdataformats/include -I.
LDFLAGS=-L${XILINX_XRT}/lib
LDLIBS=-lboost_program_options -lOpenCL

SRCS=readbin.cpp xcl2.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

KERNEL=process_data

PLATFORM=xilinx_u2_gen3x4_xdma_gc_2_202110_1
TYPE=sw_emu

all: readbin $(KERNEL).xclbin

readbin: $(OBJS)
	$(CXX) $(LDFLAGS) -o readbin $(OBJS) $(LDLIBS)

depend: .depend

$(KERNEL).xclbin: $(KERNEL).xo
	v++ -l -t $(TYPE) --platform $(PLATFORM) $(KERNEL).xo -o $(KERNEL).xclbin

$(KERNEL).xo: $(KERNEL).cpp readbin.h
	v++ -c -t $(TYPE) --platform $(PLATFORM) -k $(KERNEL) $(VPPFLAGS) $(KERNEL).cpp -o $(KERNEL).xo 

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS) $(KERNEL).xclbin $(KERNEL).xo

distclean: clean
	$(RM) *~ .depend

setup:
	# export XCL_EMULATION_MODE=$(PLATFORM)  # doesn't work called here
	emconfigutil --platform $(PLATFORM) --nd 1

include .depend
