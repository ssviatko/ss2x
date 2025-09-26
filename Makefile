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
LDFLAGS = -Wl,-rpath,./libss2x -L./libss2x -lpthread -lss2x -lstdc++exp

SS2X_TARGET = ss2x
SS2X_OBJS = ss2x.o
DT_OBJS = dispatchable_test.o
DT_TARGET = dispatchable_test
TT_OBJS = thread_test.o
TT_TARGET = thread_test
ND_OBJS = nd_test.o
ND_TARGET = nd_test
JSON_OBJS = json_test.o
JSON_TARGET = json_test
ROT_OBJS = rotator_test.o
ROT_TARGET = rotator_test
BF_TEST_OBJS = bf_test.o
BF_TEST_TARGET = bf_test
BF7_TEST_OBJS = bf7_test.o
BF7_TEST_TARGET = bf7_test
AES_TEST_OBJS = aes_test.o
AES_TEST_TARGET = aes_test
COLORTERM_TEST_OBJS = colorterm_test.o
COLORTERM_TEST_TARGET = colorterm_test

all:
	@if ! test -f $(BUILD_NUMBER_FILE); then echo 0 > $(BUILD_NUMBER_FILE); fi
	@echo $$(($$(cat $(BUILD_NUMBER_FILE)) + 1)) > $(BUILD_NUMBER_FILE)
	@if ! test -f ./libss2x/libss2x.so.1.0.0 ; then $(MAKE) -C libss2x; fi
	$(MAKE) $(SS2X_TARGET) $(DT_TARGET) $(TT_TARGET) $(ND_TARGET) $(JSON_TARGET) $(ROT_TARGET) $(BF_TEST_TARGET) $(BF7_TEST_TARGET) $(AES_TEST_TARGET) $(COLORTERM_TEST_TARGET)

$(SS2X_TARGET): $(SS2X_OBJS)

	$(LD) $(SS2X_OBJS) -o $(SS2X_TARGET) $(LDFLAGS)

$(DT_TARGET): $(DT_OBJS)

	$(LD) $(DT_OBJS) -o $(DT_TARGET) $(LDFLAGS)
	
$(TT_TARGET): $(TT_OBJS)

	$(LD) $(TT_OBJS) -o $(TT_TARGET) $(LDFLAGS)
	
$(ND_TARGET): $(ND_OBJS)

	$(LD) $(ND_OBJS) -o $(ND_TARGET) $(LDFLAGS)

$(JSON_TARGET): $(JSON_OBJS)

	$(LD) $(JSON_OBJS) -o $(JSON_TARGET) $(LDFLAGS)

$(ROT_TARGET): $(ROT_OBJS)

	$(LD) $(ROT_OBJS) -o $(ROT_TARGET) $(LDFLAGS)

$(BF_TEST_TARGET): $(BF_TEST_OBJS)

	$(LD) $(BF_TEST_OBJS) -o $(BF_TEST_TARGET) $(LDFLAGS)

$(BF7_TEST_TARGET): $(BF7_TEST_OBJS)

	$(LD) $(BF7_TEST_OBJS) -o $(BF7_TEST_TARGET) $(LDFLAGS)

$(AES_TEST_TARGET): $(AES_TEST_OBJS)

	$(LD) $(AES_TEST_OBJS) -o $(AES_TEST_TARGET) $(LDFLAGS)

$(COLORTERM_TEST_TARGET): $(COLORTERM_TEST_OBJS)

	$(LD) $(COLORTERM_TEST_OBJS) -o $(COLORTERM_TEST_TARGET) $(LDFLAGS)
	
%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CPP) $(CFLAGS) -c $<

clean:
	rm -f *.o
	rm -f *~
	rm -f $(SS2X_TARGET)
	rm -f $(DT_TARGET) 
	rm -f $(TT_TARGET)
	rm -f $(ND_TARGET)
	rm -f $(ROT_TARGET)
	rm -f $(JSON_TARGET)
	rm -f $(BF_TEST_TARGET)
	rm -f $(BF7_TEST_TARGET)
	rm -f $(AES_TEST_TARGET)
	rm -f $(COLORTERM_TEST_TARGET)
	cd libss2x && $(MAKE) clean

