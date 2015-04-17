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
#include <fcntl.h>
#include "TCP.h"

//Try to establish connection to the remote TCP server (identified by its IP and port number)
int	connect2TCPServer(const char* serverIP, const char *portNum){
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset(&hints, 0, sizeof(struct addrinfo));								//hints specifies criteria for selecting the socket address structures
    hints.ai_family = AF_UNSPEC; 											//Allow IPv4 or IPv6   
    hints.ai_socktype = SOCK_STREAM; 										//TCP socket
    hints.ai_flags = 0;														//Any protocol 
    hints.ai_protocol = 0;         

	s = getaddrinfo(serverIP, portNum, &hints, &result);					//Contains Internet addresses that can be specified for connecting

	if (s != 0) return -1;

	for (rp = result; rp != NULL; rp = rp->ai_next){						//Try each address until we successfully connect					
    	sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);   
		if (sfd == -1) continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)				
        	break;  
        close(sfd);
	}
     if (rp == NULL) return -1;												//Nothing found
     else 			 return sfd;

}

//This function tries to send data to a TCP socket; Blocks for defined period and returns if not able to send data
//Return value: number of sent bytes or -1 in case of timeout or error
ssize_t send_TCP_msg(int socket, char *buf, ssize_t msg_len){
	struct timeval timeout; 
	ssize_t sent = 0;
	ssize_t tmp;

	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	
	//Setting blocking socket send timeout in order to unblock if unable to send data for some cause
	if (setsockopt (socket, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&timeout, sizeof(timeout)) < 0)
		return -1;
	
	while(sent < msg_len){													//Keep sending message until everything is sent (TCP can crop the message) 
		tmp = send(socket, &buf[sent], msg_len - sent, MSG_NOSIGNAL);		//MSG_NOSIGNAL -> does not kill the calling process on unsucc. write
		if(tmp < 0) return -1;												//Can be caused by different sources of error, including timeout
		else sent += tmp;
	}
	return sent;
}

//This function tries to receive data from a TCP socket; Blocks for defined period and returns if no data available
//Return value:
//-2 = could not setup timeout,
//-1 = timeout, 
// 0 = assume remote server is down
ssize_t	rcv_TCP_msg(int socket, char *data, size_t maxLen, long int periodSec){
	struct timeval timeout; 
	ssize_t nread;
	 
	timeout.tv_sec = periodSec;
	timeout.tv_usec = 0;

	if (setsockopt (socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&timeout, sizeof(timeout)) < 0)
		return -2;

	nread = recv(socket, data, maxLen, 0);
	return nread;
}


//Try to establish connection to the remote TCP server (identified by its IP and port number);
//Return after defined timeout period if unable to connect
int	connect2TCPServerTimeout(const char* serverIP, const char* port, int timeout){		
	struct sockaddr_in address;  													//libc network address data structure 
    int sock = -1;         															//File descriptor for the network socket 
	int opts, portNum = atoi(port);    												//Options for socket and port number
	fd_set fdset;
    struct timeval tv;
		

	address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(serverIP); 
    address.sin_port = htons(portNum);           

    sock = socket(AF_INET, SOCK_STREAM, 0);

	
    FD_ZERO(&fdset);																//Clear out descriptor sets for select
    FD_SET(sock, &fdset);															//Add socket to the descriptor sets

    if((opts = fcntl(sock, F_SETFL, O_NONBLOCK)) < 0){
		printf("Error setting socket non-blocking!\n");		
		return -1;
	}

    int ret = connect(sock, (struct sockaddr *)&address, sizeof(address));			//if server is not ready -1 is returned

	if(ret < 0){	
		if (errno != EINPROGRESS){
		printf("Could not connect to remote server!\n");		
		return -1;
		}
	}
	
	printf("Waiting timeout\n");

    tv.tv_sec = timeout;             
    tv.tv_usec = 0;

	if ((ret = select(sock + 1, NULL, &fdset, NULL, &tv)) == 1){					//select will block until sock does not become ready or timeout expires
        int so_error;
        socklen_t len = sizeof(so_error);											//We had a positivite return, so a socket is ready

        if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0){			//SO_ERROR has information about error status of socket
			printf("Error using getsockopt\n");	
			return -1;	
		}
        if (so_error == 0){
			opts = fcntl(sock,F_GETFL);												//Everything is OK; set socket back to blocking
			opts ^= O_NONBLOCK;
			fcntl(sock,F_SETFL,opts);
			return sock;
		}	
		else{
			printf("Socket has error!\n");	
 			return -1;
		}
    }
	else if(ret == 0){																//Timeout has expired
		printf("Timeout\n");
		return -1;
	}
	else{																			//Error with select
		printf("Error with select!\n");	
		return -1;
	}
																			
}




