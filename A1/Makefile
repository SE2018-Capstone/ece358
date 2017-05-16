# Makefile
# ECE358 Computer Networks
# Yiqing Huang, 2017/05/02

CC = gcc
CFLAGS = -Wall -g  -std=c99 -D_BSD_SOURCE
LD = gcc
LDFLAGS = -g 
LDLIBS = 

NET_SRCS   	= net_util.c 
OBJS_LIB   	= net_util.o
OBJS_ADDPEER	= addpeer.o
OBJS_REMOVEPEER	= removepeer.o
TARGETS= addpeer.out removepeer.out

all: ${TARGETS}

debug: CFLAGS += -DDEBUG -g
debug: ${TARGETS}

addpeer.out: $(OBJS_LIB) $(OBJS_ADDPEER) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

removepeer.out: $(OBJS_LIB) $(OBJS_REMOVEPEER) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

%.o: %.c 
	$(CC) $(CFLAGS) -c $< 

%.d: %.c
	gcc -MM -MF $@ $<

-include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -f *.d *.o $(TARGETS) 
