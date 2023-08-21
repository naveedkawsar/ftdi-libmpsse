CC=gcc
CFLAGS=-Wall -g -L. -lmpsse

SRC=./src
INC=./inc
TEST=./test
BIN=./bin

DRIVER=$(TEST)/tests
UNITY=$(TEST)/unity

# Include project inc folder last for any mocks/stubs to be used in place
INCLUDES=-I$(UNITY) \
        -I$(INC)

UNITY_SRC=$(UNITY)/unity.c

bmi270: clean
	$(CC) main.c bmi270.c timer.c $(CFLAGS) -o bmi270; ./bmi270

clean:
	rm -f bmi270.exe