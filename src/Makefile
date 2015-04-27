OUT=../bin/
OBJECTS=./objects/
CC=g++
<<<<<<< HEAD
CFLAGS=--std=c++11 -O3 -Wall -Wextra -g -lpthread
=======
CFLAGS=-pthread --std=c++11 -O3 -Wall -Wextra -g
>>>>>>> b3dd3e73557b848b2a6383e009812b35d4d14ba2
INCLUDE_DIR = ./include/
INCLUDES += -I $(INCLUDE_DIR)
systemName = $(shell uname)
ifeq ($(systemName), Darwin)
   LIBS += -L/opt/local/lib
else
   LIBS += -luuid
endif

binary: $(OUT) $(OBJECTS) PARSINGT DBMST MMAP_FS LINENOISE
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)dbms $(OBJECTS)*.o $(LIBS) 

LINENOISE:
	$(CC) $(CFLAGS) -c $(INCLUDE_DIR)linenoise/linenoise.c -o $(OBJECTS)linenoise.o

MMAP_FS:
	make -C mmap_filesystem

OST:
	make -C os

PARSINGT:
	make -C parsing

DBMST:
	make -C dbms

test:
	make -C tests
	make -C tests run

$(OUT):
	mkdir -p $(OUT)

$(OBJECTS):
	mkdir -p $(OBJECTS)

clean:
	make -C parsing clean
	make -C os clean
	make -C dbms clean
	make -C tests clean

	rm -rf $(OBJECTS)
	rm -rf $(OUT)

.PHONY: clean
