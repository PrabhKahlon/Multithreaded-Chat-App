make: s-talk
	
s-talk: main.o
	gcc -g -o s-talk list.o main.o

testList: test.o
	gcc -g -o testList test.o list.o

test.o: main.o
	gcc -c main.c

clean: 
	rm main.o
	rm s-talk