OUT=../../bin/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall

$(OUT):
	mkdir -p $(OUT)

LinearHash.o: $(OUT) LinearHash.cpp
	$(CC) $(CFLAGS) -o $(OUT)LinearHash.o -c storage/LinearHash.cpp

LinearHash: $(OUT) LinearHash.cpp
	$(CC) $(CFLAGS) LinearHash.cpp -o $(OUT)LinearHash

run: $(OUT) LinearHash
	$(OUT)LinearHash
	
clean:
	rm -f $(OUT)LinearHash $(OUT)LinearHash.exe
