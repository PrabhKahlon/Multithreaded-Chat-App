make: s-talk
	
s-talk: main.o
	gcc -g -o s-talk list.o main.o -lpthread -Wall

main.o:
	gcc -g -c main.c

clean: 
	rm main.o
	rm s-talk