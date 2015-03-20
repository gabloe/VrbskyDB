OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)

dbms.o: $(OUT) dbms.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)dbms.o -c dbms.cpp

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)dbms.o

.PHONY: clean
