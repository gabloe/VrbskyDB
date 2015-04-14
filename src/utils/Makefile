
CC=g++
CFLAGS=--std=c++11 -g
OUT=../../bin/
EXT=

all: $(OUT) $(OUT)Reader${EXT}

$(OUT)Reader${EXT}:	Reader.cpp
	$(CC) $(CFLAGS) Reader.cpp -o $(OUT)Reader${EXT}

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f ${OUT}Reader${EXT}

