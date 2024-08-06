CFLAGS = -Wall -fPIC -O3 -std=c++23

UNAME := $(shell uname)
CC := gcc
CPP := g++
LD := g++
LDFLAGS = -lpthread -L/usr/local/lib64 -lstdc++exp -Wl,-rpath=/usr/local/lib64 -shared -Wl,-soname,libss2xfs.so.1 -rdynamic

OBJS = fs.o

all: libss2xfs

libss2xfs: $(OBJS)

	$(LD) $(OBJS) -o libss2xfs.so.1.0.0 $(LDFLAGS)
	rm -f libss2xfs.so.1
	rm -f libss2xfs.so
	ln -s libss2xfs.so.1.0.0 libss2xfs.so.1
	ln -s libss2xfs.so.1.0.0 libss2xfs.so

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CPP) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f *~
	rm -f libss2xfs.so.1.0.0
	rm -f libss2xfs.so.1
	rm -f libss2xfs.so
