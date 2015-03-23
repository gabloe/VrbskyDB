OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra
INCLUDES += -I../include/
LIBS += 

Scanner.o: $(OUT) Scanner.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)Scanner.o -c Scanner.cpp $(LIBS)

Parser.o: $(OUT) Parser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)Parser.o -c Parser.cpp $(LIBS)

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)Parser.o
	rm -f $(OUT)Scanner.o

.PHONY: clean
