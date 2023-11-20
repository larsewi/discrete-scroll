.PHONY: all clean run

all: scroll

scroll: main.o
	cc -o scroll main.o -framework ApplicationServices

main.o: main.c
	cc -g -Wall -c main.c -o main.o

clean:
	rm -f *.o
	rm -f scroll

run: scroll 
	./scroll

