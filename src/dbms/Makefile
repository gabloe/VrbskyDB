OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -g -Wall

dbms.o: $(OUT) dbms.cpp
	$(CC) $(CFLAGS) -o $(OUT)dbms.o -c dbms.cpp 

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)dbms.o
