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
#include "TCP.h"

int setRcvSocketTimeout(int sck, long int period){
	struct timeval timeout;      
	timeout.tv_sec = period;
	timeout.tv_usec = 0;
	if (setsockopt (sck, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&timeout, sizeof(timeout)) < 0)
		return -1;
	return sck;
}

int	connect2TCPServer(const char* serverIP, const char *portNum){
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = 0;
    hints.ai_protocol = 0;         

	s = getaddrinfo(serverIP, portNum, &hints, &result);					//get all possible connections

	if (s != 0) return -1;

	for (rp = result; rp != NULL; rp = rp->ai_next) {						
    	sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);   
		if (sfd == -1) continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)				//try all until successful connection
        	break;  
        close(sfd);
	}
     if (rp == NULL) return -1;												//nothing found
     else 			 return sfd;

}


ssize_t send_TCP_msg(int socket, char *buf, ssize_t msg_len){
	ssize_t sent = 0;
	ssize_t tmp;

	while(sent < msg_len){													//keep sending message (TCP can crop the message) 
		tmp = send(socket, &buf[sent], msg_len - sent, MSG_NOSIGNAL);		//MSG_NOSIGNAL -> does not die on unsucc. write
		if(tmp < 0) return -1;												//different source of error.
		else sent += tmp;
	}
	return sent;
}

//Return value -2 = could not setup timeout,
//-1 = timeout, 0 = server is down.
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

int reconnect(const char* serverIP, const char *portNum){
    int sfd;
	sfd = connect2TCPServer(serverIP, portNum);								//try to connect or return after (~20s)
				
	if(sfd != -1) return sfd;
	return -1;
}	





