CC=gcc

SRC=./src
INC=./inc
FTDI=./ftdi
TEST=./test
BIN=./bin

CFLAGS=-Wall -g -lmpsse

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux) #LINUX
    CFLAGS+=-L$(FTDI)/linux
endif

ifeq ($(OS), Windows_NT)
    CFLAGS+=-L$(FTDI)/win32
endif

DRIVER=$(TEST)/tests
UNITY=$(TEST)/unity

# Include project inc folder last for any mocks/stubs to be used in place
INCLUDES=-I$(UNITY) \
        -I$(INC)

UNITY_SRC=$(UNITY)/unity.c

bmi270: clean
	$(CC) $(SRC)/main.c $(SRC)/bmi270.c $(SRC)/nrf_delay.c $(SRC)/timer.c $(CFLAGS) -I$(INC) -I$(FTDI) -o bmi270; ./bmi270

clean:
	rm -f bmi270.exe