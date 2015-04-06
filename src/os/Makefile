
CC=g++
CFLAGS=--std=c++11 -g
OUT=../objects/

all: $(OUT) $(OUT)FileReader.o $(OUT)FileWriter.o $(OUT)File.o $(OUT)FileSystem.o

$(OUT)FileWriter.o:	FileWriter.cpp FileWriter.h
	$(CC) $(CFLAGS) -c FileWriter.cpp -o $(OUT)FileWriter.o

$(OUT)FileReader.o:	FileReader.cpp FileReader.h
	$(CC) $(CFLAGS) -c FileReader.cpp -o $(OUT)FileReader.o

$(OUT)File.o:			File.cpp File.h
	$(CC) $(CFLAGS) -c File.cpp -o $(OUT)File.o

$(OUT)FileSystem.o:	FileSystem.cpp FileSystem.h
	$(CC) $(CFLAGS) -c FileSystem.cpp -o $(OUT)FileSystem.o

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f ../objects/FileSystem.o ../objects/File.o ../objects/FileReader.o ../objects/FileWriter.o
