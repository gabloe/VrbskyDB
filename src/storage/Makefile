OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra -g
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)

all: HerpTest IterTest

HerpTest: ./HerpTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) HerpTest.cpp -oHerpTest

IterTest: ./IterTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) IterTest.cpp -oIterTest



clean:
	rm -f HerpTest
	rm -f IterTest

.PHONY: clean
