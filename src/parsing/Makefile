OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -g -Wall -Wextra

Scanner.o: $(OUT) Scanner.cpp
	$(CC) $(CFLAGS) -o $(OUT)Scanner.o -c Scanner.cpp

Parser.o: $(OUT) Parser.cpp
	$(CC) $(CFLAGS) -o $(OUT)Parser.o -c Parser.cpp

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)Parser.o
	rm -f $(OUT)Scanner.o

.PHONY: clean
