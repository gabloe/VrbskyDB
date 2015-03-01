OUT=../../bin/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall

all: Scanner.o Parser.o

Scanner.o: ./Scanner.cpp
	$(CC) $(CFLAGS) -o $(OUT)Scanner.o -c Scanner.cpp

Parser.o:
	$(CC) $(CFLAGS) -o $(OUT)Parser.o -c Parser.cpp

test: Scanner.o Parser.o
	$(CC) $(CFLAGS) $(OUT)Scanner.o $(OUT)Parser.o
	./a.out
