INCL = -I./libss2x
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
LDFLAGS = -Wl,-rpath,. -Wl,-rpath=/usr/local/lib64 -L. -lpthread -lss2x -lstdc++exp
TARGET = ss2x
OBJS = main.o

DT_OBJS = dispatchable_test.o
DT_TARGET = dispatchable_test
TT_OBJS = thread_test.o
TT_TARGET = thread_test

all: $(TARGET) $(DT_TARGET) $(TT_TARGET)

$(TARGET): $(OBJS)

	@if ! test -f $(BUILD_NUMBER_FILE); then echo 0 > $(BUILD_NUMBER_FILE); fi
	@echo $$(($$(cat $(BUILD_NUMBER_FILE)) + 1)) > $(BUILD_NUMBER_FILE)
	$(MAKE) -C libss2x/ all
	cp libss2x/libss2x.so.1.0.0 .
	rm -f libss2x.so.1
	rm -f libss2x.so
	ln -s libss2x.so.1.0.0 libss2x.so.1
	ln -s libss2x.so.1.0.0 libss2x.so
	$(LD) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(DT_TARGET): $(DT_OBJS)

	$(LD) $(DT_OBJS) -o $(DT_TARGET) $(LDFLAGS)
	
$(TT_TARGET): $(TT_OBJS)

	$(LD) $(TT_OBJS) -o $(TT_TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CPP) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f *~
	rm -f $(TARGET)
	$(MAKE) -C libss2x/ clean
	rm -f libss2x.so*
	rm dispatchable_test
	
