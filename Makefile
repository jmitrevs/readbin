CXX=g++
RM=rm -f
CPPFLAGS=-g -std=c++17 -Icnn/firmware/ap_types -I../daqdataformats/include -I../detdataformats/include -I./cnn/firmware -I/cvmfs/sft.cern.ch/lcg/releases/Boost/1.78.0-f6f04/x86_64-centos8-gcc11-opt/include
LDFLAGS= -L/cvmfs/sft.cern.ch/lcg/releases/Boost/1.78.0-f6f04/x86_64-centos8-gcc11-opt/lib
LDLIBS=-lboost_program_options

SRCS=readbin.cpp xcl2.cpp process_data.cpp cnn/firmware/vplane.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: readbin

readbin: $(OBJS)
	$(CXX) $(LDFLAGS) -o readbin $(OBJS) $(LDLIBS)

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS) $(KERNEL).xclbin $(KERNEL).xo

distclean: clean
	$(RM) *~ .depend

include .depend
