#include "thd.hpp"
#include "cmd.hpp"
#include "video_s.hpp"

#define MAX_VALUE(OLD_V, NEW_VAL) NEW_VAL>OLD_V?NEW_VAL:OLD_V

uint8_t buffer_cmd[BUFFER_CMD];
struct client_network n_clients[NUMBER_CLIENT];
struct server_network network;
uint16_t bufferToDo[32];

pthread_mutex_t mutex_client;
pthread_mutexattr_t mutexattr;

int init_client(void)
{	
    network.socket_tcp = socket(AF_INET, SOCK_STREAM, 0);
	if ( (network.socket_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    network.servaddr.sin_family = AF_INET;
    network.servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    network.servaddr.sin_port = htons(PORT);
	network.servaddr_udp.sin_family = AF_INET;
    network.servaddr_udp.sin_addr.s_addr = htonl(INADDR_ANY);
    network.servaddr_udp.sin_port = htons(PORT+1);
    network.count_clients = 0;
    network.clients = n_clients;
	network.buffer = buffer_cmd;

    int rc = bind(network.socket_tcp, (struct sockaddr *) &network.servaddr,sizeof(network.servaddr));
    if(rc<0)
    {
        printf("bind error %d (%s)\n", errno, strerror(errno));
        return MSG_ERROR;
    }

	rc = bind(network.socket_udp, (struct sockaddr *) &network.servaddr_udp,sizeof(network.servaddr));
    if(rc<0)
    {
        printf("udp bind error %d (%s)\n", errno, strerror(errno));
        return MSG_ERROR;
    }

    rc = listen(network.socket_tcp, 5);
	if (rc < 0) {
		printf( "listen: %d (%s)\n", errno, strerror(errno));
		return MSG_ERROR;
	}
    return MSG_OK;
}

extern video_streaming_c *vStreamObj_pnt;

void * listen_client(void * pv)
{
	volatile int rc = -1;
	
	socklen_t len = sizeof(network.servaddr);
	fd_set set;
	struct timeval timeout, time_select;
	int rv, max_socket = MAX_VALUE(network.socket_tcp, network.socket_udp);
	uint8_t i, k;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
	time_select.tv_sec = 1;
    time_select.tv_usec = 10;
    FD_ZERO(&set); // clear the set 
	 // add our file descriptor to the set
	uint8_t buffer[1024];
	printf("start listen sock = %d\n", network.socket_tcp); 
    while(1)
    {
		time_select.tv_sec = 1;
		//FD_ZERO(&set);
		FD_SET(network.socket_tcp, &set);
		FD_SET(network.socket_udp, &set);

		for (i = 0; i<network.count_clients; i++)
		{
			FD_SET(network.clients[i].client_socket, &set);
		} 

        rv = select(max_socket + 1, &set, NULL, NULL, &time_select);  // return the number of file description contained in the three returned sets

	    if (rv<0)
	    {
		    printf( "select: %d (%s)\n", errno, strerror(errno)); // an error accured 
	    }
	    else if(rv == 0)
	    {
    	    //printf("timeout occurred (10 second) \n"); // a timeout occured 
	    }
	    else if (FD_ISSET( network.socket_tcp , &set) && rv>0) //Add client
	    {
            if (network.count_clients<NUMBER_CLIENT)
            {
                rc = accept(network.socket_tcp, (struct sockaddr *)&network.servaddr, &len);
		        if (rc < 0) {
			    printf( "accept: %d (%s)\n", errno, strerror(errno));
                continue;
		        }
		        network.clients[network.count_clients].client_socket = rc;
				max_socket = MAX_VALUE(max_socket,rc); 
                network.count_clients++;
		        printf( "We have a new client connection! %d\n", rc);
            }
            else
            {
                printf("limit clients\n");
                rc = accept(network.socket_tcp, (struct sockaddr *)&network.servaddr, &len);
                close(rc);
            }
		    
	    }
		else if (FD_ISSET(network.socket_udp , &set) && rv>0)
		{
			len = recvfrom(network.socket_udp, (char *)buffer, MAXLINE,  MSG_WAITALL, ( struct sockaddr *) &vStreamObj_pnt->cliaddr,(unsigned int*) &vStreamObj_pnt->len_addr);
			vStreamObj_pnt->socket = network.socket_udp;  
		}
		else if (rv>0)
		{
			for (i = 0; i<network.count_clients;i++) //recieve data from clients
        	{
				if (!FD_ISSET( network.clients[i].client_socket , &set)) continue;
  				int len = read(network.clients[i].client_socket, (char *)buffer, sizeof(buffer));
  				if (len < 0)
				{		
				  		printf("read sock %d i = %d error (%s)\n",network.clients[i].client_socket, i, strerror(errno));
  				}
				if (len == 0)
				{
					if (vStreamObj_pnt->socket_tcp == network.clients[i].client_socket)
					{
						vStreamObj_pnt->socket_tcp = 0;
						vStreamObj_pnt->state = VID_STOP_SEND;
						memset(&vStreamObj_pnt->cliaddr, 0, sizeof(vStreamObj_pnt->cliaddr));
						printf("ETH: Stop stream\n");
					}
					if (max_socket == network.clients[i].client_socket)
					{
						max_socket = network.socket_tcp;
						for (uint8_t j = 0; j<network.count_clients; j++)
						{
							if (j == i) 
							{
								max_socket = MAX_VALUE(max_socket,network.socket_tcp);
								continue;
							}
							max_socket = MAX_VALUE(max_socket,network.clients[j].client_socket);
						}
					}
					printf( "Client finished %d\n",network.clients[i].client_socket);
					close(network.clients[i].client_socket);
					FD_CLR(network.clients[i].client_socket,&set);
                	if(i != network.count_clients)
                	{
                    	network.clients[i] = network.clients[network.count_clients - 1];
						network.clients[network.count_clients].client_socket = 0;
                	}
					
                	network.count_clients--;		
				}
				if (len>0)
				{
					parse_cmd(buffer,len, network.clients[i].client_socket);
				}
        	} //end for
		}//end if
    }// end while
		

}