CFLAGS = -Wall -fPIC -O3 -std=c++23

UNAME := $(shell uname)
CC := gcc
CPP := g++
LD := g++
LDFLAGS = -lpthread -Wl,-rpath=/usr/local/lib64 -shared -Wl,-soname,libss2xlog.so.1 -rdynamic

OBJS = log.o

all: libss2xlog

libss2xlog: $(OBJS)

	$(LD) $(OBJS) -o libss2xlog.so.1.0.0 $(LDFLAGS)
	rm -f libss2xlog.so.1
	rm -f libss2xlog.so
	ln -s libss2xlog.so.1.0.0 libss2xlog.so.1
	ln -s libss2xlog.so.1.0.0 libss2xlog.so

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CPP) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f *~
	rm -f libss2xlog.so.1.0.0
	rm -f libss2xlog.so.1
	rm -f libss2xlog.so
