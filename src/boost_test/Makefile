CPP=g++
OPT=-std=c++11 -O3 -Wall -Wextra
INC += -I/opt/local/include -I ../include
OUT =	CreateDocs \
	BoostTest

all: create test

create: CreateDocs.cpp
	$(CPP) $(OPT) $(INC) CreateDocs.cpp -o CreateDocs

test: BoostTest.cpp
	$(CPP) $(OPT) $(INC) BoostTest.cpp -o BoostTest

clean:
	rm -rf $(OUT)

.PHONY: clean
