OUT=../../bin/
OBJECTS=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)

all: $(OUT)LinearHashTest $(OUT)ParserTest $(OUT)UUIDTest

run: $(OUT)LinearHashTest $(OUT)ParserTest $(OUT)UUIDTest
	cd $(OUT) && ./ParserTest && ./LinearHashTest && ./UUIDTest

$(OUT)LinearHashTest: $(OUT) ./LinearHashTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./LinearHashTest.cpp -o $(OUT)LinearHashTest

$(OUT)ParserTest: $(OUT) ./ParserTest.cpp
	cd ../parsing/ && $(MAKE) Parser.o && $(MAKE) Scanner.o
	$(CC) $(CFLAGS) $(INCLUDES) ./ParserTest.cpp -o $(OUT)ParserTest $(OBJECTS)Parser.o $(OBJECTS)Scanner.o
	cp queries $(OUT)queries

$(OUT)UUIDTest: $(OUT) ./uuidtest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./uuidtest.cpp -o $(OUT)UUIDTest

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)UUIDTest
	rm -f $(OUT)LinearHashTest
	rm -f $(OUT)ParserTest
	rm -f $(OUT)queries

.PHONY: clean
