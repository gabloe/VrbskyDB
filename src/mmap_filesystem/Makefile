OUT=./
OBJECTS=./
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra -g
INCLUDES += 
LIBS +=

binary: filesystem.o fstest.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)FSTest $(OBJECTS)*.o $(LIBS) 

fstest.o: FSTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c FSTest.cpp -o$(OUT)fstest.o
	
filesystem.o: Filesystem.cpp Filesystem.h
	$(CC) $(CFLAGS) $(INCLUDES) -c Filesystem.cpp -o$(OUT)filesystem.o

clean:
	rm -f *.o *.db
	rm -f FSTest

.PHONY: clean
