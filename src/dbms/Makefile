OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -g -Wall -Wextra
INCLUDES=-I../include/

dbms.o: $(OUT) dbms.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)dbms.o -c dbms.cpp 

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)dbms.o

.PHONY: clean
