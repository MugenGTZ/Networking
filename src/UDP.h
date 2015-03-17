#ifndef _UDP_H
#define _UDP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

int  	createUDPSocket			();
int  	createUDPTimeoutSocket	(long int seconds);
bool 	bindToSocket			(int sck, unsigned short portNum);
void 	setBroadcastSocket		(int sck, struct sockaddr_in *broadcastAddr,const char* subnetBcast, socklen_t addrlen, unsigned short portNum);
void 	setReceiver				(struct sockaddr_in *remoteAddrStruct, socklen_t addrlen, const char* remoteIP, unsigned short portNum);

#endif
