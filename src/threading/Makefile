OUT=../objects/
CC=g++
CFLAGS=--std=c++11 -O3 -Wall -Wextra -g
INCLUDE_DIR=../include/
INCLUDES=-I$(INCLUDE_DIR)
OBJECTS=

all: $(OUT) $(OBJECTS)

test: PoolTest.cpp
	$(CC) $(CFLAGS) $(INCLUDES) PoolTest.cpp -o PoolTest

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(OBJECTS)

.PHONY: clean
