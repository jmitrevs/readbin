CXX=g++
RM=rm -f
CPPFLAGS=-g -std=c++17 -I../daqdataformats/include -I../detdataformats/include -I${XILINX_XRT}/include
LDFLAGS=-L${XILINX_XRT}/lib
LDLIBS=-lboost_program_options -lOpenCL

SRCS=readbin.cpp xcl2.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

PLATFORM=xilinx_u2_gen3x4_xdma_gc_2_202110_1
TYPE=sw_emu

all: readraw

readraw: $(OBJS)
	$(CXX) $(LDFLAGS) -o readbin $(OBJS) $(LDLIBS)

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) *~ .depend

include .depend
