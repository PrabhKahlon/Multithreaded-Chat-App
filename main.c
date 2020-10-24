//main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#define MAX_THREADS 4

void *userInput(void *threadID)
{
    //this should be dynamically allocated eventually using realloc.
    int* tempId = (int*)threadID;
    printf("This is thread %d\n", *tempId);
    sleep(5);
    printf("Still good :)\n");
    pthread_exit(NULL);
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
        exit(1);
    }

    pthread_t threads[MAX_THREADS];
    int temp = 0;
    int newThread = pthread_create(&threads[temp], NULL, userInput, (void *)&temp);

    char userMessage[1000];

    while(strcmp(userMessage, "!"))
    {
        fgets(userMessage, 1000, stdin);
        int tempLength = strlen(userMessage);
        if(tempLength > 0 && userMessage[tempLength - 1] == '\n')
        {
            userMessage[tempLength - 1] = '\0';
        }
        if(strlen(userMessage) > 0)
        {
            //printf("%s\n", userMessage);
        }
    }

    return 0;
}