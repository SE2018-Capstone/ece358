# Makefile
# ECE358 Computer Networks
# Yiqing Huang, 2017/05/02

CC = gcc
CFLAGS = -Wall -g  -std=c99 -D_BSD_SOURCE
LD = gcc
LDFLAGS = -g 
LDLIBS = 

NET_SRCS   = net_util.c 
OBJS_LIB   = net_util.o
OBJS_TS	   = ts.o
OBJS_TC	   = tc.o
OBJS_US	   = us.o
OBJS_UC	   = uc.o
TARGETS= ts.out tc.out us.out uc.out 

all: ${TARGETS}

ts.out: $(OBJS_LIB) $(OBJS_TS) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

tc.out: $(OBJS_LIB) $(OBJS_TC) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

us.out: $(OBJS_LIB) $(OBJS_US) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

uc.out: $(OBJS_LIB) $(OBJS_UC) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

%.o: %.c 
	$(CC) $(CFLAGS) -c $< 

%.d: %.c
	gcc -MM -MF $@ $<

-include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -f *.d *.o $(TARGETS) 
