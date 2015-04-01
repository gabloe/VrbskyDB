
CMP=g++
FLAGS=--std=c++11 -g
OBJECTS=FileWriter.o FileReader.o File.o FileSystem.o
T_OBJECTS=FileSystemTest.o FileSystemCreateTest.o EndianTest.o

all:	FileReader.o FileWriter.o File.o FileSystem.o

FileWriter.o:	FileWriter.cpp FileWriter.h
	$(CMP) $(FLAGS) -c FileWriter.cpp

FileReader.o:	FileReader.cpp FileReader.h
	$(CMP) $(FLAGS) -c FileReader.cpp

File.o:			File.cpp File.h
	$(CMP) $(FLAGS) -c File.cpp

FileSystem.o:	FileSystem.cpp FileSystem.h
	$(CMP) $(FLAGS) -c FileSystem.cpp

FileSystemTest.o:	FileSystemTest.cpp
	$(CMP)	$(FLAGS) -c FileSystemTest.cpp

FileSystemCreateTest.o:	FileSystemCreateTest.cpp
	$(CMP)	$(FLAGS) -c FileSystemCreateTest.cpp

EndianTest.o:	EndianTest.cpp
	$(CMP)	$(FLAGS) -c EndianTest.cpp

test: $(OBJECTS) $(T_OBJECTS)
	rm -f test.dat
	$(CMP) $(FLAGS) FileWriter.o FileSystemTest.o FileReader.o FileSystem.o File.o -o FSTest
	$(CMP) $(FLAGS) FileWriter.o FileSystemCreateTest.o FileReader.o FileSystem.o File.o -o FSMTest
	$(CMP) $(FLAGS) EndianTest.o -o EndianTest


clean:
	rm -f *.o FSMTest FSTest test.dat
