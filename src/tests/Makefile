OUT=../../bin/
OBJECTS=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra

all: LinearHashTest ParserTest

LinearHashTest: $(OUT) ./LinearHashTest.cpp
	$(CC) $(CFLAGS) ./LinearHashTest.cpp -o $(OUT)LinearHashTest

ParserTest: $(OUT) ./ParserTest.cpp
	cd ../parsing && $(MAKE) Parser.o && $(MAKE) Scanner.o
	$(CC) $(CFLAGS) ./ParserTest.cpp -o $(OUT)ParserTest $(OBJECTS)Parser.o $(OBJECTS)Scanner.o
	cp queries $(OUT)queries

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)LinearHashTest
	rm -f $(OUT)ParserTest
	rm -f $(OUT)queries

.PHONY: clean
