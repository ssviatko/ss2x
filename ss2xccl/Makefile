CFLAGS = -Wall -g -fPIC -O3 -std=c++23

UNAME := $(shell uname)
CC := gcc
CPP := g++
LD := g++
LDFLAGS = -lpthread -Wl,-rpath=/usr/local/lib64 -shared -Wl,-soname,libss2xccl.so.1 -rdynamic
DLDFLAGS = -lpthread -Wl,-rpath=/usr/local/lib64

OBJS = ccl.o dispatchable.o
DOBJS = dispatchable_test.o ccl.o dispatchable.o

all: libss2xccl dispatchable_test

libss2xccl: $(OBJS)

	$(LD) $(OBJS) -o libss2xccl.so.1.0.0 $(LDFLAGS)
	rm -f libss2xccl.so.1
	rm -f libss2xccl.so
	ln -s libss2xccl.so.1.0.0 libss2xccl.so.1
	ln -s libss2xccl.so.1.0.0 libss2xccl.so

dispatchable_test: $(DOBJS)

	$(LD) $(DOBJS) -o dispatchable_test $(DLDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CPP) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f *~
	rm -f libss2xccl.so.1.0.0
	rm -f libss2xccl.so.1
	rm -f libss2xccl.so
	rm -f dispatchable_test

