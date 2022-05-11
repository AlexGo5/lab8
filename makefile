COMPILER=g++

CFLAGS=-W -Wall -pedantic -Wextra -Wno-unused-parameter -pthread -lpthread

.PHONY:all
all: start

start: main.o

main.o: main.cpp
	$(COMPILER) $(CFLAGS) main.cpp -o main

clean:
	rm -rf *.o main