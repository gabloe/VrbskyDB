OUT=../../bin/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall

all: LinearHashTest

LinearHashTest: $(OUT) ./LinearHashTest.cpp
	$(CC) $(CFLAGS) ./LinearHashTest.cpp -o $(OUT)LinearHashTest

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)LinearHashTest
