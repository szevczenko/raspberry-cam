
// Server side implementation of UDP client-server model 
extern "C"{

#include <limits.h>
#include <config.h>
}
#include "thd.hpp"
#include "video_s.hpp"
#include "cmd.hpp"
#include "motor_pwm.hpp"
#define test_errno(msg) do{if (errno) {perror(msg); exit(EXIT_FAILURE);}} while(0)


video_streaming_c *vStreamObj_pnt;
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
	video_streaming_c vStreamObj;
	vStreamObj_pnt = &vStreamObj;
	vStreamObj.start_process();

	vStreamObj.wait_to_end();
	pthread_join(listen, NULL);
	#if !CONFIG_PLATFORM_LINUX
	pthread_join(com, NULL);
	#endif
	pthread_attr_destroy(&attr);
	      
    return 0; 
} 
