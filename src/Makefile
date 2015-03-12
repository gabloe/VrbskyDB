OUT=../bin/
OBJECTS=./objects/
CC=g++
CFLAGS=--std=c++11 -O3 -g -Wall -Wextra
INCLUDES += -I ./include/

binary: $(OUT) $(OBJECTS)dbms.o $(OBJECTS)Parser.o $(OBJECTS)Scanner.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)dbms $(OBJECTS)*.o 

$(OBJECTS)Scanner.o: ./parsing/Scanner.cpp
	cd ./parsing && $(MAKE) Scanner.o

$(OBJECTS)Parser.o: ./parsing/Parser.cpp 
	cd ./parsing && $(MAKE) Parser.o

$(OBJECTS)dbms.o: ./dbms/dbms.cpp
	cd ./dbms && $(MAKE) dbms.o

test:
	cd ./tests && $(MAKE) all

$(OUT):
	mkdir -p $(OUT)

clean:
	cd ./tests && $(MAKE) clean
	rm -rf $(OBJECTS)
	rm -rf $(OUT)

.PHONY: clean
