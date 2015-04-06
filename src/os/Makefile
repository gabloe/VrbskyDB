
CC=g++
CFLAGS=--std=c++11 -g
OBJ=../objects/

all:	$(OBJ)FileReader.o $(OBJ)FileWriter.o $(OBJ)File.o $(OBJ)FileSystem.o

$(OBJ)FileWriter.o:	FileWriter.cpp FileWriter.h
	$(CC) $(CFLAGS) -c FileWriter.cpp -o $(OBJ)FileWriter.o

$(OBJ)FileReader.o:	FileReader.cpp FileReader.h
	$(CC) $(CFLAGS) -c FileReader.cpp -o $(OBJ)FileReader.o

$(OBJ)File.o:			File.cpp File.h
	$(CC) $(CFLAGS) -c File.cpp -o $(OBJ)File.o

$(OBJ)FileSystem.o:	FileSystem.cpp FileSystem.h
	$(CC) $(CFLAGS) -c FileSystem.cpp -o $(OBJ)FileSystem.o

clean:
	rm -f ../objects/FileSystem.o ../objects/File.o ../objects/FileReader.o ../objects/FileWriter.o
