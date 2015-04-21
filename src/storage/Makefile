OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra -g
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)

HerpTest: ./HerpTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) HerpTest.cpp -oHerpTest


clean:
	rm -f HerpTest

.PHONY: clean
