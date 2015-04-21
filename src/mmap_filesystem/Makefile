OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O0 -Wall -Wextra -g
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)
OBJECTS=$(OUT)mmap_filesystem.o


$(OUT)mmap_filesystem.o: $(OUT) Filesystem.cpp Filesystem.h
	$(CC) $(CFLAGS) $(INCLUDES) -c Filesystem.cpp -o$(OUT)mmap_filesystem.o

test: ReadTest WriteTest CreateTest FSReader HerpTest

FSReader: FilesystemReader.cpp $(OUT)mmap_filesystem.o
	$(CC) $(CFLAGS) $(INCLUDES) FilesystemReader.cpp $(OBJECTS) -oFSReader

CreateTest: CreateTest.cpp $(OUT)mmap_filesystem.o
	$(CC) $(CFLAGS) $(INCLUDES) CreateTest.cpp $(OBJECTS) -oCreateTest

ReadTest: ReadTest.cpp $(OUT)mmap_filesystem.o
	$(CC) $(CFLAGS) $(INCLUDES) ReadTest.cpp $(OBJECTS) -oReadTest

WriteTest: WriteTest.cpp $(OUT)mmap_filesystem.o
	$(CC) $(CFLAGS) $(INCLUDES) WriteTest.cpp $(OBJECTS) -oWriteTest

HerpTest: HerpTest.cpp $(OUT)mmap_filesystem.o
	$(CC) $(CFLAGS) $(INCLUDES) HerpTest.cpp $(OBJECTS) -oHerpTest

$(OUT):
	mkdir $(OUT)

clean:
	rm -f $(OBJECTS)
	rm -f *.db
	rm -f ReadTest
	rm -f WriteTest
	rm -f CreateTest
	rm -f FSReader
	rm -f HerpTest

.PHONY: clean
