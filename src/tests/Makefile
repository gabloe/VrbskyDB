OUT=../../bin/
OBJECTS=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra
INCLUDES=-I../include/

all: $(OUT)LinearHashTest $(OUT)ParserTest $(OUT)BPlusTreeTest

run: $(OUT)LinearHashTest $(OUT)ParserTest
	cd $(OUT) && ./ParserTest && ./LinearHashTest

$(OUT)BPlusTreeTest: $(OUT) ./BPlusTreeTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./BPlusTreeTest.cpp -o $(OUT)BPlusTreeTest

$(OUT)LinearHashTest: $(OUT) ./LinearHashTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./LinearHashTest.cpp -o $(OUT)LinearHashTest

$(OUT)ParserTest: $(OUT) ./ParserTest.cpp
	cd ../parsing/ && $(MAKE) Parser.o && $(MAKE) Scanner.o
	$(CC) $(CFLAGS) $(INCLUDES) ./ParserTest.cpp -o $(OUT)ParserTest $(OBJECTS)Parser.o $(OBJECTS)Scanner.o
	cp queries $(OUT)queries

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)LinearHashTest
	rm -f $(OUT)ParserTest
	rm -f $(OUT)queries

.PHONY: clean
