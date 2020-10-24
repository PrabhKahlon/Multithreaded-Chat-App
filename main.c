//main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "list.h"
#define MAX_THREADS 4

/*this was used to test thread creation no longer needed
void *userInput(void *threadID)
{
    int* tempId = (int*)threadID;
    printf("This is thread %d\n", *tempId);
    sleep(5);
    printf("Still good :)\n");
    pthread_exit(NULL);
}
*/

/*
* Created messageList which handles all outgoing messages. So far I got 2 threads one for keyboard input and the other just prints out the latest message 
* from the message list onto the screen after 10 seconds then exists. This is just some experimental code to get used to working with threads and stuff. Real
* grind starts this weekend. Still need to figure out UDP and socket stuff. 
*/

List* messageList;

void* waitForDatagram()
{
    pthread_exit(NULL);
}

void* printToScreen()
{
    printf("This is the printing thread :). After 10 seconds it will print the latest message in messageList\n");
    sleep(10);
    void *messageToPrint = List_curr(messageList);
    printf("Latest string held in the message list is : %s\n", (char*)messageToPrint);
    printf("exiting thread\n");
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

    messageList = List_create();

    pthread_t threads[MAX_THREADS];
    int temp = 0;
    //int newThread = pthread_create(&threads[temp], NULL, userInput, (void *)&temp);
    int newThread = pthread_create(&threads[temp], NULL, printToScreen, NULL);

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
            List_add(messageList, userMessage);
        }
    }

    return 0;
}