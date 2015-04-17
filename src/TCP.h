#ifndef _TCP_H
#define _TCP_H

int			connect2TCPServer		(const char* serverIP, const char *portNum);				//Tries to set the TCP connection to a remote Server
int			connect2TCPServerTimeout(const char* serverIP, const char* port, int timeout);		//Tries to set the TCP connection to a remote Server; gives up after timeout seconds		
ssize_t	 	send_TCP_msg			(int socket, char *buf, ssize_t msg_len);					//Sends a message via TCP socket 
ssize_t		rcv_TCP_msg				(int socket, char *data, size_t maxLen, long int periodSec);//Receives a message via TCP socket; 
																								//Returns if timeout has elapsed

#endif
