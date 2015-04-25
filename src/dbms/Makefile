OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra -g
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)
OBJECTS=$(OUT)dbms.o	\
	$(OUT)aggregator.o

all: $(OUT) $(OBJECTS)

$(OUT)dbms.o: dbms.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)dbms.o -c dbms.cpp

$(OUT)aggregator.o: Aggregator.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)aggregator.o -c Aggregator.cpp

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OBJECTS)

.PHONY: clean
