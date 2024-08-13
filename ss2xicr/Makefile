CFLAGS = -Wall -g -fPIC -O3 -std=c++23

UNAME := $(shell uname)
CC := gcc
CPP := g++
LD := g++
LDFLAGS = -lpthread -Wl,-rpath=/usr/local/lib64 -shared -Wl,-soname,libss2xicr.so.1 -rdynamic

OBJS = icr.o

all: libss2xicr

libss2xicr: $(OBJS)

	$(LD) $(OBJS) -o libss2xicr.so.1.0.0 $(LDFLAGS)
	rm -f libss2xicr.so.1
	rm -f libss2xicr.so
	ln -s libss2xicr.so.1.0.0 libss2xicr.so.1
	ln -s libss2xicr.so.1.0.0 libss2xicr.so

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CPP) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f *~
	rm -f libss2xicr.so.1.0.0
	rm -f libss2xicr.so.1
	rm -f libss2xicr.so
