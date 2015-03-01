OUT=../../bin/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall

LinearHash.o: LinearHash.cpp
	$(CC) $(CFLAGS) -o $(OUT)LinearHash.o -c storage/LinearHash.cpp

LinearHash: LinearHash.cpp
	$(CC) $(CFLAGS) LinearHash.cpp -o $(OUT)LinearHash

run: LinearHash
	$(OUT)LinearHash
	
clean:
	rm -f $(OUT)LinearHash $(OUT)LinearHash.exe
