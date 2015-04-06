
CMP=g++
FLAGS=--std=c++11 -g
OBJECTS=FileWriter.o FileReader.o File.o FileSystem.o
T_OBJECTS=CreateTest.o EndianTest.o FileTest.o WriteTest.o ReadTest.o LargeWriteTest.o \
		  LargeReadTest.o ReadAllTest.o ReplaceTest.o WriteReadTest.o

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

ReplaceTest.o:	ReplaceTest.cpp
	$(CMP) $(FLAGS) -c ReplaceTest.cpp

LargeWriteTest.o:	LargeWriteTest.cpp
	$(CMP) $(FLAGS) -c LargeWriteTest.cpp

ReadTest.o:	ReadTest.cpp
	$(CMP) $(FLAGS) -c ReadTest.cpp

ReadAllTest.o:	ReadAllTest.cpp
	$(CMP) $(FLAGS) -c ReadAllTest.cpp

LargeReadTest.o:	LargeReadTest.cpp
	$(CMP) $(FLAGS) -c LargeReadTest.cpp

EndianTest.o:	EndianTest.cpp
	$(CMP) $(FLAGS) -c EndianTest.cpp

WriteReadTest.o:	WriteReadTest.cpp
	$(CMP) $(FLAGS) -c WriteReadTest.cpp

test: $(OBJECTS) $(T_OBJECTS)
	rm -f test.dat LWTest EndianTest CTest WTest RTest LRTest
	$(CMP) $(FLAGS) CreateTest.o $(OBJECTS) -o CTest
	$(CMP) $(FLAGS) FileTest.o   $(OBJECTS) -o FTest
	$(CMP) $(FLAGS) WriteTest.o  $(OBJECTS) -o WTest
	$(CMP) $(FLAGS) ReplaceTest.o  $(OBJECTS) -o RepTest
	$(CMP) $(FLAGS) ReadTest.o   $(OBJECTS) -o RTest
	$(CMP) $(FLAGS) ReadAllTest.o   $(OBJECTS) -o RAllTest
	$(CMP) $(FLAGS) LargeWriteTest.o   $(OBJECTS) -o LWTest
	$(CMP) $(FLAGS) LargeReadTest.o   $(OBJECTS) -o LRTest
	$(CMP) $(FLAGS) WriteReadTest.o   $(OBJECTS) -o WRTest
	$(CMP) $(FLAGS) EndianTest.o -o EndianTest


clean:
	rm -f *.db *.o test.dat EndianTest CTest WTest RTest RepTest FTest LWTest LRTest RAllTest WRTest
