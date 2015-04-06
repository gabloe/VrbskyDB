OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra
INCLUDES += -I../include/
LIBS += 
OBJS=$(OUT)Scanner.o $(OUT)Parser.o

all: $(OUT) $(OBJS)

$(OUT)Scanner.o: Scanner.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)Scanner.o -c Scanner.cpp $(LIBS)

$(OUT)Parser.o: Parser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)Parser.o -c Parser.cpp $(LIBS)

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)Parser.o
	rm -f $(OUT)Scanner.o

.PHONY: clean
