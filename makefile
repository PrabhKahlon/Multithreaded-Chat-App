make: s-talk
	
s-talk: main.o
	gcc -g -o s-talk list.o main.o -lpthread -Wall

testList: test.o
	gcc -g -o testList test.o list.o -Wall

test.o: main.o
	gcc -c main.c -Wall

clean: 
	rm main.o
	rm s-talk