OUT=../bin/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall

all: LinearHash Parser Scanner

$(OUT):
	mkdir -p $(OUT)

LinearHash: $(OUT) ./storage/LinearHash.cpp
	$(CC) $(CFLAGS) -o $(OUT)LinearHash.o -c storage/LinearHash.cpp

Scanner: $(OUT) ./parsing/Scanner.cpp
	$(CC) $(CFLAGS) -o $(OUT)Scanner.o -c parsing/Scanner.cpp

Parser: $(OUT) ./parsing/Parser.cpp 
	$(CC) $(CFLAGS) -o $(OUT)Parser.o -c parsing/Parser.cpp

clean:
	rm -r $(OUT) 
