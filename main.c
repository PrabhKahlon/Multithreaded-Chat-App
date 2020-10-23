//main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void userInput()
{
    //this should be dynamically allocated eventually using realloc.
    char message[200];
    while(1)
    {
        scanf("%s", message);
        if(strlen(message) > 0)
        {
            printf("does this work?");
        }
    }
}

int main(int argc, char** argv)
{
    //Take in command line arguments in format:
    //s-talk [my port number] [remote machine name] [remote port number]
    if(argc == 4)
    {
        int myPort = atoi(argv[1]);
        char* remoteMachine = argv[2];
        int remotePort = atoi(argv[3]);

        printf("My Port: %d, Other Machine Name: %s, Other Port: %d\n", myPort, remoteMachine, remotePort);

    }
    else
    {
        printf("Invalid arguments, please supply arguments in this format: s-talk [my port number] [remote machine name] [remote port number]\n");
    }

    pthread_t testThread;
    userInput();

    return 0;
}