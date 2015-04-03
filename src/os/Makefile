
CMP=g++
FLAGS=--std=c++11 -g
OBJECTS=FileWriter.o FileReader.o File.o FileSystem.o
T_OBJECTS=CreateTest.o EndianTest.o FileTest.o WriteTest.o ReadTest.o

all:	FileReader.o FileWriter.o File.o FileSystem.o

FileWriter.o:	FileWriter.cpp FileWriter.h
	$(CMP) $(FLAGS) -c FileWriter.cpp

FileReader.o:	FileReader.cpp FileReader.h
	$(CMP) $(FLAGS) -c FileReader.cpp

File.o:			File.cpp File.h
	$(CMP) $(FLAGS) -c File.cpp

FileSystem.o:	FileSystem.cpp FileSystem.h
	$(CMP) $(FLAGS) -c FileSystem.cpp

CreateTest.o:	CreateTest.cpp
	$(CMP) $(FLAGS) -c CreateTest.cpp

FileTest.o:		FileTest.cpp
	$(CMP) $(FLAGS) -c FileTest.cpp

WriteTest.o:	WriteTest.cpp
	$(CMP) $(FLAGS) -c WriteTest.cpp

ReadTest.o:	ReadTest.cpp
	$(CMP) $(FLAGS) -c ReadTest.cpp

EndianTest.o:	EndianTest.cpp
	$(CMP) $(FLAGS) -c EndianTest.cpp

test: $(OBJECTS) $(T_OBJECTS)
	rm -f test.dat EndianTest CTest WTest RTest
	$(CMP) $(FLAGS) CreateTest.o FileSystem.o File.o -o CTest
	$(CMP) $(FLAGS) FileTest.o FileSystem.o File.o -o FTest
	$(CMP) $(FLAGS) WriteTest.o FileWriter.o FileSystem.o File.o -o WTest
	$(CMP) $(FLAGS) ReadTest.o FileReader.o FileSystem.o File.o -o RTest
	$(CMP) $(FLAGS) EndianTest.o -o EndianTest


clean:
	rm -f *.o test.dat EndianTest CTest WTest RTest FTest
