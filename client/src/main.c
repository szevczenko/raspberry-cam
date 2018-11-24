
// Client side implementation of UDP client-server model 
#include "MQTTLinux.h"
#include "linux/input.h"
#include <limits.h>
#include <pthread.h>

#include "cmd.h"

#define MSG_OK 0
#define MSG_ERROR -1
#define MSG_TIMEOUT 1

#define PORT     8080 
#define MAXLINE 1024 
#define SERVER_ADR "127.0.0.1"



/*
struct server_conect
{
    int socket;
    struct sockaddr_in servaddr; 
}server_connect; */
// Driver code 


pthread_mutex_t mutex_client;
pthread_mutexattr_t mutexattr;


Network scon;


int main() { 
    int ret = MSG_ERROR;
    
    NetworkInit(&scon);
    ret = NetworkConnect(&scon,SERVER_ADR,PORT);
    if (ret != MSG_OK)
    {
        printf("NetworkConnect error\r\n");
        return MSG_ERROR;
    }
    
    ////////////


	pthread_t	keyboard, com;
	pthread_attr_t	attr;
	size_t size = PTHREAD_STACK_MIN;
	
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex_client, &mutexattr);
	errno = pthread_attr_init(&attr);
	printf("size = %lu\n",size);
	if (errno) {
		perror("pthread_attr_init");
		return EXIT_FAILURE;
	}
	pthread_attr_getstacksize(&attr, &size);
	pthread_create(&keyboard, &attr, keyboard_thd, (void *)&scon);
	//pthread_create(&com, &attr, doTelnet, NULL);

	pthread_join(keyboard, NULL);
	//pthread_join(com, NULL);
	pthread_attr_destroy(&attr);
    ////////////
    
            
    //ret = scon.mqttwrite(&scon,"HELLO FROM CLIENT!!!\n\r",22,1000);
    //printf("Send data ret = %d", ret);

    return 0;
} 

/////////////////////////////////////////////////////////////////////////////
