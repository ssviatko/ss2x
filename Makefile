INCL = -I./ss2xlog -I./ss2xicr -I./ss2xdata -I./ss2xfs
BUILD_NUMBER_FILE = build.no
RELEASE_NUMBER_FILE = release.no
BUILD_DATE=$$(date +'%Y-%m-%d %H:%M %z %Z')
BUILD_NUMBER=$$(cat $(BUILD_NUMBER_FILE))
RELEASE_NUMBER=$$(cat $(RELEASE_NUMBER_FILE))
CFLAGS = -DBUILD_NUMBER="\"$(BUILD_NUMBER)\"" -DBUILD_DATE="\"$(BUILD_DATE)\"" -DRELEASE_NUMBER="\"$(RELEASE_NUMBER)\"" -std=c++23 -Wall -O3 $(INCL)
UNAME = $(shell uname)
CC = gcc
CPP = g++
LD = g++
LDFLAGS = -Wl,-rpath,. -Wl,-rpath=/usr/local/lib64 -L. -lpthread -lss2xlog -lss2xicr -lss2xdata -lss2xfs
TARGET = ss2x
OBJS = main.o

all: $(TARGET)

$(TARGET): $(OBJS)

	@if ! test -f $(BUILD_NUMBER_FILE); then echo 0 > $(BUILD_NUMBER_FILE); fi
	@echo $$(($$(cat $(BUILD_NUMBER_FILE)) + 1)) > $(BUILD_NUMBER_FILE)
	$(MAKE) -C ss2xlog/ all
	cp ss2xlog/libss2xlog.so.1.0.0 .
	rm -f libss2xlog.so.1
	rm -f libss2xlog.so
	ln -s libss2xlog.so.1.0.0 libss2xlog.so.1
	ln -s libss2xlog.so.1.0.0 libss2xlog.so
	$(MAKE) -C ss2xicr/ all
	cp ss2xicr/libss2xicr.so.1.0.0 .
	rm -f libss2xicr.so.1
	rm -f libss2xicr.so
	ln -s libss2xicr.so.1.0.0 libss2xicr.so.1
	ln -s libss2xicr.so.1.0.0 libss2xicr.so
	$(MAKE) -C ss2xdata/ all
	cp ss2xdata/libss2xdata.so.1.0.0 .
	rm -f libss2xdata.so.1
	rm -f libss2xdata.so
	ln -s libss2xdata.so.1.0.0 libss2xdata.so.1
	ln -s libss2xdata.so.1.0.0 libss2xdata.so
	$(MAKE) -C ss2xfs/ all
	cp ss2xfs/libss2xfs.so.1.0.0 .
	rm -f libss2xfs.so.1
	rm -f libss2xfs.so
	ln -s libss2xfs.so.1.0.0 libss2xfs.so.1
	ln -s libss2xfs.so.1.0.0 libss2xfs.so
	$(LD) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CPP) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f *~
	rm -f $(TARGET)
	$(MAKE) -C ss2xlog/ clean
	rm -f libss2xlog.so*
	$(MAKE) -C ss2xicr/ clean
	rm -f libss2xicr.so*
	$(MAKE) -C ss2xdata/ clean
	rm -f libss2xdata.so*
	$(MAKE) -C ss2xfs/ clean
	rm -f libss2xfs.so*
	
