

all:	File.o FileSystem.o

File.o:
	g++ --std=c++11 -c File.cpp

FileSystem.o:
	g++ --std=c++11 -c FileSystem.cpp

test: all
	g++ --std=c++11 FileSystemTest.cpp FileSystem.o File.o -o FSTest

clean:
	rm -f *.o FSTest
