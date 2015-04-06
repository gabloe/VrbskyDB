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

binary: $(OUT) $(OBJECTS)dbms.o $(OBJECTS)Parser.o $(OBJECTS)Scanner.o $(OBJECTS)FileSystem.o $(OBJECTS)FileReader.o $(OBJECTS)FileWriter.o $(OBJECTS)File.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)dbms $(OBJECTS)*.o $(LIBS) 

$(OBJECTS)Scanner.o: ./parsing/Scanner.cpp
	cd ./parsing && $(MAKE) Scanner.o

$(OBJECTS)FileSystem.o: ./os/FileSystem.cpp
	cd ./os && $(MAKE) FileSystem.o
	mv ./os/FileSystem.o $(OBJECTS)FileSystem.o

$(OBJECTS)FileWriter.o: ./os/FileWriter.cpp
	cd ./os && $(MAKE) FileWriter.o
	mv ./os/FileWriter.o $(OBJECTS)FileWriter.o

$(OBJECTS)FileReader.o: ./os/FileReader.cpp
	cd ./os && $(MAKE) FileReader.o
	mv ./os/FileReader.o $(OBJECTS)FileReader.o

$(OBJECTS)File.o: ./os/File.cpp
	cd ./os && $(MAKE) File.o
	mv ./os/File.o $(OBJECTS)File.o

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
