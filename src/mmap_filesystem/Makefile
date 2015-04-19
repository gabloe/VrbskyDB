OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra -g
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)
OBJECTS=$(OUT)mmap_filesystem.o

mmap_filesystem.o: Filesystem.cpp Filesystem.h
	$(CC) $(CFLAGS) $(INCLUDES) -c Filesystem.cpp -o$(OUT)mmap_filesystem.o

test: readtest writetest

readtest: ReadTest.cpp mmap_filesystem.o
	$(CC) $(CFLAGS) $(INCLUDES) ReadTest.cpp $(OBJECTS) -oReadTest

writetest: WriteTest.cpp mmap_filesystem.o
	$(CC) $(CFLAGS) $(INCLUDES) WriteTest.cpp $(OBJECTS) -oWriteTest

clean:
	rm -f $(OBJECTS)
	rm -f *.db
	rm -f ReadTest
	rm -f WriteTest

.PHONY: clean
