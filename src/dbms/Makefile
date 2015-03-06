OUT=../../bin/
CC=g++
CFLAGS=--std=c++11 -O3 -g -Wall

all: dbms.o

$(OUT):
	mkdir -p $(OUT)

dbms.o: $(OUT) dbms.cpp
	$(CC) $(CFLAGS) $(OUT)LinearHash.o $(OUT)Parser.o $(OUT)Scanner.o -o $(OUT)dbms.o -c dbms.cpp

test: $(OUT) dbms.o
	$(CC) $(CFLAGS) $(OUT)LinearHash.o $(OUT)Parser.o $(OUT)Scanner.o $(OUT)dbms.o
