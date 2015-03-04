OUT=../../bin/
CC=g++
CFLAGS=--std=c++11 -O3 -g -Wall

all: Scanner.o Parser.o

$(OUT):
	mkdir -p $(OUT)

Scanner.o: $(OUT) Scanner.cpp
	$(CC) $(CFLAGS) -o $(OUT)Scanner.o -c Scanner.cpp

Parser.o: $(OUT) Parser.cpp
	$(CC) $(CFLAGS) -o $(OUT)Parser.o -c Parser.cpp

test: Scanner.o Parser.o
	$(CC) $(CFLAGS) $(OUT)Scanner.o $(OUT)Parser.o
	./a.out
