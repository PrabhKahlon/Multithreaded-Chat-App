make: all
	
all: main.o
	gcc -g -o s-talk list.o main.o -lpthread -Wall
	rm main.o

main.o:
	gcc -g -c main.c

clean:
	rm s-talk