#ifndef _TCP_H
#define _TCP_H

int			connect2TCPServer		(const char* serverIP, const char *portNum);				//setup TCP connection to a remote Server		
ssize_t	 	send_TCP_msg			(int socket, char *buf, ssize_t msg_len);
ssize_t		rcv_TCP_msg				(int socket, char *data, size_t maxLen, long int periodSec);//socket, buffer, expected length and timeout period; 
																								//returns number of received bytes or error indicators
#endif
