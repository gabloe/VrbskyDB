OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra -g
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)
OBJECTS=$(OUT)mmap_filesystem.o

mmap_filesystem.o: Filesystem.cpp Filesystem.h
	$(CC) $(CFLAGS) $(INCLUDES) -c Filesystem.cpp -o$(OUT)mmap_filesystem.o

test: FSTest.cpp mmap_filesystem.o
	$(CC) $(CFLAGS) $(INCLUDES) FSTest.cpp $(OBJECTS) -ommaptest

clean:
	rm -f $(OBJECTS)
	rm -f *.db
	rm -f mmaptest

.PHONY: clean
