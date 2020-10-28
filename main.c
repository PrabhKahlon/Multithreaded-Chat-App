//main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "list.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#define MAX_THREADS 4

/*
* Sending and receiving message sent over the network using udp now works. Need to solve the critical section problem.
*TODO
*-create mutex to allow only a single read or write operation to each list at one time. (Nathan, try to do this ty :D)
*-change harcoded values
*-test
*PROBLEMS
*-since we dont have a mutex threads are trying to access the stream at the same time. This sometimes causes the message to not be received unless you type something and press enter.
* can reproduce this by taking out the newline character from "printf("Message received: %s\n", buf);" on line 157.
*/

List* sendList;
List* receiveList;
char* myPort = 0;
char* remoteMachine;
char* remotePort = 0;

//Set up send mutex and receive mutex intializing
pthread_mutex_t sendMutex = PTHREAD_MUTEX_INITIALIZER;
//Receive Mutex
pthread_mutex_t receiveMutex = PTHREAD_MUTEX_INITIALIZER;



void* printToScreen()
{
    //this thread will print the message to the screen. Need to implement.
    pthread_exit(NULL);
}

void* takeInput()
{

    //this thread takes input from the user and adds it to the sendList.

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
            //lock and unlock send mutex while taking input 
            pthread_mutex_lock(&sendMutex);
            List_add(sendList, userMessage);
            List_add(sendList, "TEST");
            pthread_mutex_unlock(&sendMutex);
        }
    }

    pthread_exit(NULL);
}

//Thread that sends a message. Right now it just sends it to itself but it uses the udp protocol and is over the network. This was aids. :) :) :)
//Everything is hard coded rn I will change this to work with command line arguments on Sunday or Monday. Should be easy.
//Receiveing a message is slightly different. Reading this function should give you a good idea about how it works or ignore it idc.
void* sendMessage()
{
    
    //Variables to store socket status and info
    int socketStatus = 0;
    int sendSocket = 0;
    struct addrinfo aInfo;
    struct addrinfo *results;

    //Allocate memory for addrinfo and set up the struct
    memset(&aInfo, 0, sizeof(aInfo));
    aInfo.ai_family = AF_INET;
    aInfo.ai_socktype = SOCK_DGRAM;

    //Get address info and create a socket
    socketStatus = getaddrinfo(remoteMachine, myPort, &aInfo, &results);
    if(socketStatus != 0)
    {
        fprintf(stderr, "Failed to create a socket\n");
        exit(1);
    }
    sendSocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    if(sendSocket == -1)
    {
        fprintf(stderr, "Failed to create a socket\n");
        exit(1);

    }


    while(1) 
    {
        if (List_count(sendList) != 0)
        {
            printf("%s\n", (char*)(List_trim(sendList)));
        }
    }
    //Temporary message to send
    char buf[] = "Hello World!";

    //Send the message.
    //lock the send mutex while sending a message
    pthread_mutex_lock(&sendMutex);
    int sentMessageSize = sendto(sendSocket, buf, strlen(buf), 0, results->ai_addr, results->ai_addrlen);
    if(sentMessageSize == -1)
    {
        fprintf(stderr, "Failed to send message\n");
        exit(1);
    }

    //Free the memory taken by the struct earlier
    freeaddrinfo(results);
    //unlock mutex after message is sent
    pthread_mutex_unlock(&sendMutex);
    printf("Message sent from thread\n");
    //Close the socket after all sending is done
    close(sendSocket);

    pthread_exit(NULL);
}

void* receiveMesssage()
{
    //Variables to store socket status and info
    int socketStatus = 0;
    int receiveSocket = 0;
    struct addrinfo aInfo;
    struct addrinfo *results;
    //Allocate memory for addrinfo and set up the struct
    memset(&aInfo, 0, sizeof(aInfo));
    aInfo.ai_family = AF_INET;
    aInfo.ai_socktype = SOCK_DGRAM;

    //Get address info, create a socket then bind it.
    socketStatus = getaddrinfo(remoteMachine, remotePort, &aInfo, &results);
    if(socketStatus != 0)
    {
        fprintf(stderr, "Failed to create a socket\n");
        exit(1);
    }
    receiveSocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    if(receiveSocket == -1)
    {
        fprintf(stderr, "Failed to create a socket\n");
        exit(1);
    }
    int bindStatus = bind(receiveSocket, results->ai_addr, results->ai_addrlen);
    if(bindStatus == -1)
    {
        fprintf(stderr, "Failed to bind socket\n");
        exit(1);
    }
    //Free the memory taken by the struct earlier
    freeaddrinfo(results);
    //Variables needed to receive the message
    struct sockaddr_storage remoteAddress;
    socklen_t addressLength;

    //The buffer for the message to be stored
    char buf[1000];

    //Receive messages from the socket
    
    addressLength = sizeof(remoteAddress);
    int receiveMessageLength = recvfrom(receiveSocket, buf, 999, 0, (struct sockaddr*)&remoteAddress, &addressLength);
    
    if(receiveMessageLength == -1)
    {
        fprintf(stderr, "Failed to receive message\n");
        exit(1);
    }
    buf[receiveMessageLength] = '\0';
    printf("Message received: %s\n", buf); // removed \n
    
    //Close the socket after message has been received and exit- thread.
    close(receiveSocket);
    pthread_exit(NULL);
}



int main(int argc, char** argv)
{
    //Take in command line arguments in format:
    //s-talk [my port number] [remote machine name] [remote port number]

    if(argc == 4)
    {
        myPort = argv[1];
        remoteMachine = argv[2];
        remotePort = argv[3];

        printf("My Port: %s, Other Machine Name: %s, Other Port: %s\n", myPort, remoteMachine, remotePort);

    }
    else
    {
        fprintf(stderr, "Invalid arguments, please supply arguments in this format: s-talk [my port number] [remote machine name] [remote port number]\n");
        exit(1);
    }

    //Test to check if Mutex is properly initialized
    if (pthread_mutex_init(&sendMutex, NULL) != 0 || pthread_mutex_init(&receiveMutex, NULL) != 0) 
    {
        fprintf(stderr, "Mutex Not Initialized");
        return 1;
    }


    //Create the sendList. This is useless at the moment.
    sendList = List_create();
    receiveList = List_create();

    //Create the 4 threads needed for the assignment.
    pthread_t threads[MAX_THREADS];
    int temp = 0;
    //int newThread = pthread_create(&threads[temp], NULL, userInput, (void *)&temp);
    int newThread = pthread_create(&threads[0], NULL, printToScreen, NULL);
    //printf("%d\n", newThread);
    int keyboardThread = pthread_create(&threads[1], NULL, takeInput, NULL);
    //printf("%d\n", keyboardThread);
    int receiveThread = pthread_create(&threads[2], NULL, receiveMesssage, NULL);
    //printf("%d\n", receiveThread);
    int sendThread = pthread_create(&threads[3], NULL, sendMessage, NULL);
    //printf("%d\n", sendThread);

    //wait for keyboard thread to finish then exit main and program.
    pthread_join(threads[1], NULL);
    return 0;
}