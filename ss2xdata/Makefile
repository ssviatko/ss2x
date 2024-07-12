CFLAGS = -Wall -fPIC -O3
CCFLAGS = -Wall -Wno-maybe-uninitialized -fPIC -O3 -std=c++23

UNAME := $(shell uname)
CC := gcc
CPP := g++
LD := g++
LDFLAGS = -lpthread -Wl,-rpath=/usr/local/lib64 -shared -Wl,-soname,libss2xdata.so.1 -rdynamic

OBJS = data.o md5.o sha1.o sha2.o

all: libss2xdata

libss2xdata: $(OBJS)

	$(LD) $(OBJS) -o libss2xdata.so.1.0.0 $(LDFLAGS)
	rm -f libss2xdata.so.1
	rm -f libss2xdata.so
	ln -s libss2xdata.so.1.0.0 libss2xdata.so.1
	ln -s libss2xdata.so.1.0.0 libss2xdata.so

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CPP) $(CCFLAGS) -c $<

clean:
	rm -f *.o
	rm -f *~
	rm -f libss2xdata.so.1.0.0
	rm -f libss2xdata.so.1
	rm -f libss2xdata.so
