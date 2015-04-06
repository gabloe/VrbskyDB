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

binary: $(OUT) $(OBJECTS) OS PARSING DBMS
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)dbms $(OBJECTS)*.o $(LIBS) 

OS:
	make -C os

PARSING:
	make -C parsing

DBMS:
	make -C dbms

test:
	make -C tests
	make -C tests run

$(OUT):
	mkdir -p $(OUT)

$(OBJECTS):
	mkdir -p $(OBJECTS)

clean:
	cd ./tests && $(MAKE) clean
	rm -rf $(OBJECTS)
	rm -rf $(OUT)

.PHONY: clean
