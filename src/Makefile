OUT=../bin/
OBJECTS=./objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra

binary: $(OUT) obj
	$(CC) $(CFLAGS) -o $(OUT)dbms $(OBJECTS)*.o 

obj:  dbms.o Parser.o Scanner.o

Scanner.o: ./parsing/Scanner.cpp
	cd ./parsing && $(MAKE) Scanner.o

Parser.o: ./parsing/Parser.cpp 
	cd ./parsing && $(MAKE) Parser.o

dbms.o: ./dbms/dbms.cpp
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
