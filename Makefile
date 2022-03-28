CXX=g++
RM=rm -f
CPPFLAGS=-g -std=c++17 -I../daqdataformats/include -I../detdataformats/include
LDLIBS=-lboost_program_options

SRCS=readbin.cpp process.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

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
