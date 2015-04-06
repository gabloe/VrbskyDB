OUT=../../bin/
OBJECTS=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra -g
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)

all: $(OUT)LinearHashTest $(OUT)ParserTest $(OUT)UUIDTest \
	$(OUT)WriteReadTest $(OUT)EndianTest $(OUT)WriteTest \
	$(OUT)ReadTest $(OUT)LargeReadTest $(OUT)LargeWriteTest \
	$(OUT)CreateTest $(OUT)FileTest $(OUT)ReplaceTest $(OUT)ReadAllTest

run: $(OUT)LinearHashTest $(OUT)ParserTest $(OUT)UUIDTest
	cd $(OUT) && ./ParserTest && ./LinearHashTest && ./UUIDTest

RapidJSONTest: $(OUT)RapidJSONTest

ParserTest: $(OUT)ParserTest

$(OUT)RapidJSONTest: $(OUT) ./rapidjsontest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./rapidjsontest.cpp -o $(OUT)RapidJSONTest

$(OUT)LinearHashTest: $(OUT) ./LinearHashTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./LinearHashTest.cpp -o $(OUT)LinearHashTest

$(OUT)ParserTest: $(OUT) ./ParserTest.cpp
	cd ../parsing/ && $(MAKE) Parser.o && $(MAKE) Scanner.o
	$(CC) $(CFLAGS) $(INCLUDES) ./ParserTest.cpp -o $(OUT)ParserTest $(OBJECTS)Parser.o $(OBJECTS)Scanner.o
	cp queries $(OUT)queries

$(OUT)UUIDTest: $(OUT) ./uuidtest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./uuidtest.cpp -o $(OUT)UUIDTest

$(OUT)WriteReadTest: $(OUT) ./WriteReadTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./WriteReadTest.cpp -o $(OUT)WriteReadTest

$(OUT)EndianTest: $(OUT) ./EndianTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./EndianTest.cpp -o $(OUT)EndianTest

$(OUT)LargeReadTest: $(OUT) ./LargeReadTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./LargeReadTest.cpp -o $(OUT)LargeReadTest

$(OUT)ReadAllTest: $(OUT) ./ReadAllTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./ReadAllTest.cpp -o $(OUT)ReadAllTest

$(OUT)ReadTest: $(OUT) ./ReadTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./ReadTest.cpp -o $(OUT)ReadTest
	
$(OUT)LargeWriteTest: $(OUT) ./LargeWriteTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./LargeWriteTest.cpp -o $(OUT)LargeWriteTest

$(OUT)WriteTest: $(OUT) ./WriteTest.cp
	$(CC) $(CFLAGS) $(INCLUDES) ./WriteTest.cpp -o $(OUT)WriteTest

$(OUT)ReplaceTest: $(OUT) ./ReplaceTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./ReplaceTest.cpp -o $(OUT)ReplaceTest

$(OUT)FileTest: $(OUT) ./FileTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./FileTest.cpp -o $(OUT)FileTest

$(OUT)CreateTest: $(OUT) ./CreateTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./CreateTest.cpp -o $(OUT)CreateTest

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)UUIDTest
	rm -f $(OUT)LinearHashTest
	rm -f $(OUT)ParserTest
	rm -f $(OUT)queries

.PHONY: clean
