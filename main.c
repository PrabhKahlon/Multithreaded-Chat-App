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
#define MAX_BUFFER 1024

/*
* s-talk
* Name: Prabhjot Kahlon
* Student Number: 301350905
* Name: Nathan Kee
* Student Number: 301328767
* Reference: Beej's guide to network programming, specifically http://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch, http://beej.us/guide/bgnet/html/#socket 
* and http://beej.us/guide/bgnet/html/#sendtorecv. Used these to get a basic understanding of how sockets and udp sendto recvfrom functions work since we've never 
* done network programming before.
*/

List *sendList;
List *receiveList;
char *myPort = 0;
char *remoteMachine;
char *remotePort = 0;
bool isLocalShutdown = false;
bool isRemoteShutdown = false;
pthread_t threads[MAX_THREADS];

//Send mutex and condition
pthread_mutex_t sendMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sendCond = PTHREAD_COND_INITIALIZER;
//Receive Mutex and condition
pthread_mutex_t receiveMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t receiveCond = PTHREAD_COND_INITIALIZER;

//function to begin shutdown of threads
// !isLocalShutdown && !isRemoteShutdown
void shutdownLocal()
{
    isLocalShutdown = true;
    pthread_cond_signal(&sendCond);
    pthread_cond_signal(&receiveCond);
}

void shutdownRemote()
{
    isRemoteShutdown = true;
    pthread_cond_signal(&sendCond);
    pthread_cond_signal(&receiveCond);
}

//this thread will print the message to the screen.
void *printToScreen()
{
    while (!isLocalShutdown && !isRemoteShutdown)
    {
        //Lock mutex at receiving list
        pthread_mutex_lock(&receiveMutex);
        pthread_cond_wait(&receiveCond, &receiveMutex);
        if (List_count(receiveList) != 0)
        {
            //remove the item from the list and print it out
            char *messageToPrint = (char *)(List_trim(receiveList));
            //check if it is the shutdown command
            if (strcmp(messageToPrint, "!") == 0)
            {
                fputs(messageToPrint, stdout);
                fputs("\n", stdout);
                shutdownRemote();
            }
            else
            {
                fputs(messageToPrint, stdout);
                fputs("\n", stdout);
            }
            free(messageToPrint);
        }
        //unlock the mutex
        pthread_mutex_unlock(&receiveMutex);
    }
    printf("exiting print thread\n");
    pthread_exit(NULL);
}

//this thread takes input from the user and adds it to the sendList.
void *takeInput()
{
    char userMessage[MAX_BUFFER] = "";

    while (strcmp(userMessage, "!") && !isLocalShutdown && !isRemoteShutdown)
    {
        fgets(userMessage, MAX_BUFFER, stdin);
        int tempLength = strlen(userMessage);
        if (tempLength > 0 && userMessage[tempLength - 1] == '\n')
        {
            userMessage[tempLength - 1] = '\0';
        }

        if (strlen(userMessage) > 0)
        {
            //lock and unlock send mutex while taking input
            pthread_mutex_lock(&sendMutex);
            char *sendBuffer = malloc(sizeof(userMessage));
            strcpy(sendBuffer, userMessage);
            List_add(sendList, sendBuffer);
            pthread_cond_signal(&sendCond);
            pthread_mutex_unlock(&sendMutex);
        }
    }

    shutdownLocal();
    pthread_exit(NULL);
}

//Thread that sends a message.
void *sendMessage()
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
    if (socketStatus != 0)
    {
        fprintf(stderr, "Failed to get address information\n");
        exit(1);
    }
    sendSocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    if (sendSocket == -1)
    {
        fprintf(stderr, "Failed to create a send socket\n");
        exit(1);
    }

    //Send the message.
    //lock the send mutex while sending a message
    while (!isLocalShutdown && !isRemoteShutdown)
    {
        pthread_mutex_lock(&sendMutex);
        pthread_cond_wait(&sendCond, &sendMutex);
        if (List_count(sendList) > 0)
        {
            char *buffer = (char *)List_trim(sendList);
            int sentMessageSize = sendto(sendSocket, buffer, strlen(buffer), 0, results->ai_addr, results->ai_addrlen);
            if (sentMessageSize == -1)
            {
                fprintf(stderr, "Failed to send message\n");
                exit(1);
            }
            //Free the memory taken up by the buffer.
            free(buffer);
        }
        //unlock mutex after message is sent
        pthread_mutex_unlock(&sendMutex);
    }
    //Free the memory taken by the struct earlier
    freeaddrinfo(results);

    //Close the socket after all sending is done[]
    close(sendSocket);

    pthread_exit(NULL);
}

//Thread that receives a message.
void *receiveMesssage()
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
    if (socketStatus != 0)
    {
        fprintf(stderr, "Failed to get address information\n");
        exit(1);
    }
    receiveSocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    if (receiveSocket == -1)
    {
        fprintf(stderr, "Failed to create a receive socket\n");
        exit(1);
    }
    int bindStatus = bind(receiveSocket, results->ai_addr, results->ai_addrlen);
    if (bindStatus == -1)
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
    char receivedMessage[MAX_BUFFER];

    //Receive messages from the socket

    while (!isLocalShutdown && !isRemoteShutdown)
    {
        addressLength = sizeof(remoteAddress);
        int receiveMessageLength = recvfrom(receiveSocket, receivedMessage, MAX_BUFFER - 1, 0, (struct sockaddr *)&remoteAddress, &addressLength);

        if (receiveMessageLength == -1)
        {
            fprintf(stderr, "Failed to receive message\n");
            exit(1);
        }
        receivedMessage[receiveMessageLength] = '\0';
        pthread_mutex_lock(&receiveMutex);
        char *receiveBuffer = malloc(sizeof(receivedMessage));
        strcpy(receiveBuffer, receivedMessage);
        List_add(receiveList, receiveBuffer);
        pthread_cond_signal(&receiveCond);
        pthread_mutex_unlock(&receiveMutex);
    }

    //Close the socket after message has been received and exit- thread.
    close(receiveSocket);

    pthread_exit(NULL);
}

void freeList(void* pItem)
{
    free(pItem);
}

//main
int main(int argc, char **argv)
{
    //Take in command line arguments in format:
    //s-talk [my port number] [remote machine name] [remote port number]

    if (argc == 4)
    {
        myPort = argv[1];
        remoteMachine = argv[2];
        remotePort = argv[3];

        //For testing
        printf("My Port: %s, Other Machine Name: %s, Other Port: %s\n", myPort, remoteMachine, remotePort);
    }
    else
    {
        fprintf(stderr, "Invalid arguments, please supply arguments in this format: s-talk [my port number] [remote machine name] [remote port number]\n");
        return 1;
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
    int printThread = pthread_create(&threads[0], NULL, printToScreen, NULL);
    int keyboardThread = pthread_create(&threads[1], NULL, takeInput, NULL);
    int receiveThread = pthread_create(&threads[2], NULL, receiveMesssage, NULL);
    int sendThread = pthread_create(&threads[3], NULL, sendMessage, NULL);

    //wait for threads to finish then exit main and program.
    pthread_join(threads[0], NULL);
    if(isLocalShutdown)
    {
        pthread_join(threads[1], NULL);
        pthread_cancel(threads[2]);
    }
    else
    {
        pthread_cancel(threads[1]);
        pthread_join(threads[3], NULL);
    }
    
    pthread_join(threads[3], NULL);

    pthread_mutex_destroy(&sendMutex);
    pthread_mutex_destroy(&receiveMutex);

    List_free(sendList, freeList);
    List_free(receiveList, freeList);

    return 0;
}