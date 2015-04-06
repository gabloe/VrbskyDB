OUT=../bin/
OBJECTS=./objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra -g
INCLUDES += -I ./include/
OS = $(shell uname)
ifeq ($(OS), Darwin)
   LIBS += -L/opt/local/lib -lreadline
else
   LIBS += -luuid -lreadline
endif

binary: $(OUT) $(OBJECTS)dbms.o $(OBJECTS)Parser.o $(OBJECTS)Scanner.o OS 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)dbms $(OBJECTS)*.o $(LIBS) 

$(OBJECTS)Scanner.o: ./parsing/Scanner.cpp
	cd ./parsing && $(MAKE) Scanner.o

OS:
	make -C os

$(OBJECTS)Parser.o: ./parsing/Parser.cpp 
	cd ./parsing && $(MAKE) Parser.o

$(OBJECTS)dbms.o: ./dbms/dbms.cpp
	cd ./dbms && $(MAKE) dbms.o

test:
	cd ./tests && $(MAKE) all

$(OUT):
	mkdir -p $(OUT)

clean:
	cd ./tests && $(MAKE) clean
	rm -rf $(OBJECTS)
	rm -rf $(OUT)

.PHONY: clean
