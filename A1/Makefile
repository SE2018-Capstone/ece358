# Makefile
# ECE358 Computer Networks
# Yiqing Huang, 2017/05/02

CC = g++
CPPFLAGS = -Wall -g  -std=c++0x -D_BSD_SOURCE
LD = g++
LDFLAGS = -g 
LDLIBS = 

OBJS_LIB   	= net_util.o
OBJS_ADDPEER	= addpeer.o
OBJS_REMOVEPEER	= removepeer.o
TARGETS= addpeer.out removepeer.out

all: ${TARGETS}

debug: CPPFLAGS += -DDEBUG -g
debug: ${TARGETS}

addpeer.out: $(OBJS_LIB) $(OBJS_ADDPEER) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

removepeer.out: $(OBJS_LIB) $(OBJS_REMOVEPEER) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< 

%.d: %.cpp
	gcc -MM -MF $@ $<

-include $(SRCS:.cpp=.d)

.PHONY: clean
clean:
	rm -f *.d *.o $(TARGETS) 
