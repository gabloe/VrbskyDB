OUT=../../bin/
OBJECTS=../objects/
CC=g++
UNAME := $(shell uname -s)
ifeq ($(UNAME), Darwin)
CFLAGS=-std=c++11 -O3 -Wall -Wextra -g
else
CFLAGS=-std=c++11 -O3 -Wall -Wextra -g -luuid
endif
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)
OS_OBJS=$(OBJECTS)mmap_filesystem.o

OUTPUT=$(OUT)ParserTest $(OUT)BulkInsert $(OUT)Insert $(OUT)EndianTest \
	$(OUT)CreateTest $(OUT)FileTest $(OUT)WriteReadTest \
	$(OUT)WriteTest \

NOT_WORKING=$(OUT)LinearHashTest \
		   	$(OUT)LargeWriteTest $(OUT)LargeReadTest \
			$(OUT)ReadAllTest $(OUT)ReplaceTest

all: $(OUT) PARSING OS $(OUTPUT)


run: all
	for B in $(OUTPUT); do	\
		rm -f test.dat;		\
		echo "Running $$B";	\
		$$B;				\
	done

PARSING:
	make -C ../parsing/

OS:
	make -C ../mmap_filesystem/

RapidJSONTest: $(OUT)RapidJSONTest

ParserTest: $(OUT)ParserTest

$(OUT)RapidJSONTest: ./rapidjsontest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./rapidjsontest.cpp -o $(OUT)RapidJSONTest

$(OUT)LinearHashTest: ./LinearHashTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./LinearHashTest.cpp -o $(OUT)LinearHashTest

$(OUT)ParserTest: ./ParserTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./ParserTest.cpp -o $(OUT)ParserTest $(OBJECTS)Parser.o $(OBJECTS)Scanner.o
	cp queries $(OUT)queries

$(OUT)UUIDTest: ./uuidtest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./uuidtest.cpp -o $(OUT)UUIDTest

$(OUT)WriteReadTest: ./WriteReadTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(OS_OBJS) ./WriteReadTest.cpp -o $(OUT)WriteReadTest

$(OUT)EndianTest: ./EndianTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(OS_OBJS) ./EndianTest.cpp -o $(OUT)EndianTest

$(OUT)LargeReadTest: ./LargeReadTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(OS_OBJS) ./LargeReadTest.cpp -o $(OUT)LargeReadTest

$(OUT)ReadAllTest: ./ReadAllTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(OS_OBJS) ./ReadAllTest.cpp -o $(OUT)ReadAllTest

$(OUT)LargeWriteTest: ./LargeWriteTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(OS_OBJS) ./LargeWriteTest.cpp -o $(OUT)LargeWriteTest

$(OUT)WriteTest: ./WriteTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(OS_OBJS) ./WriteTest.cpp -o $(OUT)WriteTest

$(OUT)ReplaceTest: ./ReplaceTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(OS_OBJS) ./ReplaceTest.cpp -o $(OUT)ReplaceTest

$(OUT)FileTest: ./FileTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(OS_OBJS) ./FileTest.cpp -o $(OUT)FileTest

$(OUT)CreateTest: ./CreateTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(OS_OBJS) ./CreateTest.cpp -o $(OUT)CreateTest

$(OUT)Insert: ./Insert.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./Insert.cpp -o $(OUT)Insert

$(OUT)BulkInsert: ./BulkInsert.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ./BulkInsert.cpp -o $(OUT)BulkInsert

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUTPUT)
	rm -f *.dat

.PHONY: clean
