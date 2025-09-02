CCFLAGS = -Wall -Wno-maybe-uninitialized -fPIC -O3 -std=c++23
CFLAGS = -Wall -fPIC -O3
UNAME := $(shell uname)
CC := gcc
CPP := g++
LD := g++
LDFLAGS = -lpthread -Wl,-rpath=/usr/local/lib64 -shared -Wl,-soname,libss2x.so.1 -rdynamic -lstdc++exp

OBJS = aes.o ccl.o dispatchable.o bf.o data.o md5.o sha1.o sha2.o hmac.o fs.o icr.o log.o doubletime.o nd.o json.o
TESTCASESOBJS = log-xterm-256colors.o

all: libss2x test-cases

test-cases: $(TESTCASESOBJS)
	$(LD) log-xterm-256colors.o -o log-xterm-256colors

libss2x: $(OBJS)

	$(LD) $(OBJS) -o libss2x.so.1.0.0 $(LDFLAGS)
	rm -f libss2x.so.1
	rm -f libss2x.so
	ln -s libss2x.so.1.0.0 libss2x.so.1
	ln -s libss2x.so.1.0.0 libss2x.so
	strip libss2x.so.1.0.0

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CPP) $(CCFLAGS) -c $<

clean:
	rm -f *.o
	rm -f *~
	rm -f libss2x.so.1.0.0
	rm -f libss2x.so.1
	rm -f libss2x.so
	rm -f log-xterm-256colors

