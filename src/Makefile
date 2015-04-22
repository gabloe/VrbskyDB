OUT=../bin/
OBJECTS=./objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra
INCLUDES += -I ./include/
systemName = $(shell uname)
ifeq ($(systemName), Darwin)
   LIBS += -L/opt/local/lib -lreadline
else
   LIBS += -luuid -lreadline
endif

binary: $(OUT) $(OBJECTS) PARSINGT DBMST MMAP_FS
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)dbms $(OBJECTS)*.o $(LIBS) 

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
