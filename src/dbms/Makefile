OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra 
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)
OBJECTS=$(OUT)dbms.o

all: $(OUT) $(OBJECTS)

$(OUT)dbms.o: dbms.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)dbms.o -c dbms.cpp

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OBJECTS)

.PHONY: clean
