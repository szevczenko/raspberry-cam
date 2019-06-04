
// Server side implementation of UDP client-server model 
#include "thd.h"
#include <limits.h>
#include <config.h>
#include "cmd.h"
#include "motor_pwm.h"

#define test_errno(msg) do{if (errno) {perror(msg); exit(EXIT_FAILURE);}} while(0)

// Driver code 
int main() { 
	int ret = MSG_ERROR;
	pthread_t	listen, com;
	pthread_attr_t	attr;
	size_t size = PTHREAD_STACK_MIN;
	#if !CONFIG_PLATFORM_LINUX
	init_pwm();
	#endif
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
	init_client();
	pthread_create(&listen, &attr, listen_client, NULL);
	#if !CONFIG_PLATFORM_LINUX
	pthread_create(&com, &attr, go_cmd_thd, NULL);
	#endif

	pthread_join(listen, NULL);
	#if !CONFIG_PLATFORM_LINUX
	pthread_join(com, NULL);
	#endif
	pthread_attr_destroy(&attr);
	      
    return 0; 
} 
