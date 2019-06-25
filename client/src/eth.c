/*******************************************************************************
 * Copyright (c) 2014, 2017 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *    Ian Craggs - return codes from linux_read_tcp
 *******************************************************************************/

#include "eth.h"
#include <errno.h>

void TimerInit(Timer* timer)
{
	timer->end_time = (struct timeval){0, 0};
}

char TimerIsExpired(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
	timeradd(&now, &interval, &timer->end_time);
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout, 0};
	timeradd(&now, &interval, &timer->end_time);
}


int TimerLeftMS(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}


int linux_read_tcp(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
	{
		interval.tv_sec = 0;
		interval.tv_usec = 100;
	}

	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

	int bytes = 0;
	while (bytes < len)
	{
		int rc = recv(n->my_socket, &buffer[bytes], (size_t)(len - bytes), 0);
		if (rc == -1)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			  bytes = -1;
			break;
		}
		else if (rc == 0)
		{
			bytes = 0;
			break;
		}
		else
			bytes += rc;
	}
	return bytes;
}

int linux_read_udp(Network* n, unsigned char* buffer, int len, int *addr_len)
{
	int bytes;
	bytes = recvfrom(n->my_socket_udp, (char*)buffer, len, MSG_CONFIRM, (struct sockaddr*)&n->servaddr_udp, addr_len); 
	return bytes;
}


int linux_write_tcp(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	struct timeval tv;

	tv.tv_sec = 0;  /* 30 Secs Timeout */
	tv.tv_usec = timeout_ms * 1000;  // Not init'ing this can cause strange errors

	setsockopt(n->my_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(struct timeval));
	int	rc = write(n->my_socket, buffer, len);
	return rc;
}

int linux_write_udp(Network* n, unsigned char* buffer, int len)
{
	int rc = -1;
	rc = sendto(n->my_socket_udp, (const char*)buffer, len, 0, (const struct sockaddr*)&n->servaddr_udp, sizeof(n->servaddr_udp)); 
	return rc;
}

void NetworkInit(Network* n)
{
	n->my_socket = 0;
	n->my_socket_udp = 0;
	n->read_tcp = linux_read_tcp;
	n->write_tcp = linux_write_tcp;
	n->read_udp = linux_read_udp;
	n->write_udp = linux_write_udp;
}


int NetworkConnect(Network* n, char* addr, int port)
{
	int type = SOCK_STREAM;
	struct sockaddr_in address;
	int rc = -1;
	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL; 
	struct addrinfo hints = {0, AF_INET, SOCK_STREAM, 0, 0, NULL, NULL, NULL}; //AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP,

	if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)
	{
		struct addrinfo* res = result;

		/* prefer ip4 addresses */
		while (res)
		{
			if (res->ai_family == AF_INET)
			{
				result = res;
				break;
			}
			res = res->ai_next;
		}

		if (result->ai_family == AF_INET)
		{
			address.sin_port = htons(port);
			address.sin_family = family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else
		{
			rc = -1;
			printf("result->ai_family != AF_INET\r\n");
		}
			

		freeaddrinfo(result);
	}
	else
	{
		printf("getaddrinfo error\r\n");
	}
	if (rc == 0)
	{
		n->my_socket = socket(family, type, 0);
		if (n->my_socket != -1)
			rc = connect(n->my_socket, (struct sockaddr*)&address, sizeof(address));
		else
			rc = -1;
	}

	if ((n->my_socket_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        printf("socket creation failed"); 
        exit(-1); 
    } 

	memset(&n->servaddr_udp, 0, sizeof(n->servaddr_udp)); 
  
    // Filling server information 
    n->servaddr_udp.sin_family = AF_INET; 
    n->servaddr_udp.sin_port = htons(port + 1); 
    n->servaddr_udp.sin_addr.s_addr = inet_addr(addr); 

	return rc;
}


void NetworkDisconnect(Network* n)
{
	close(n->my_socket);
}
