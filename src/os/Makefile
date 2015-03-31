
CMP=g++
FLAGS=--std=c++11 -g

all:	FileReader.o FileWriter.o File.o FileSystem.o

FileWriter.o:
	$(CMP) $(FLAGS) -c FileWriter.cpp

FileReader.o:
	$(CMP) $(FLAGS) -c FileReader.cpp

File.o:
	$(CMP) $(FLAGS) -c File.cpp

FileSystem.o:
	$(CMP) $(FLAGS) -c FileSystem.cpp

test: all
	$(CMP) $(FLAGS) FileWriter.o FileSystemTest.cpp FileSystem.o File.o -o FSTest

clean:
	rm -f *.o FSTest test.dat
