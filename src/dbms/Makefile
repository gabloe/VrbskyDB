OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra -g
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)
OS = $(shell uname)
ifeq ($(OS), Darwin)
   LIBS += 
else
   LIBS += -luuid
endif

dbms.o: $(OUT) dbms.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT)dbms.o -c dbms.cpp $(LIBS)

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OUT)dbms.o

.PHONY: clean
