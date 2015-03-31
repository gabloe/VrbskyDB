

all:	FileReader.o FileWriter.o File.o FileSystem.o

FileWriter.o:
	g++ --std=c++11 -c FileWriter.cpp

FileReader.o:
	g++ --std=c++11 -c FileReader.cpp

File.o:
	g++ --std=c++11 -c File.cpp

FileSystem.o:
	g++ --std=c++11 -c FileSystem.cpp

test: all
	g++ --std=c++11 FileWriter.o FileSystemTest.cpp FileSystem.o File.o -o FSTest

clean:
	rm -f *.o FSTest
