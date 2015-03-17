#include "UDP.h"

int createUDPSocket(){
	int sck;

	if ((sck = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
		perror("cannot create socket\n"); 
		exit(EXIT_FAILURE);
	}
	return sck; 
}

int createUDPTimeoutSocket(long int seconds){
	int sck;
	struct timeval tv;

	tv.tv_sec = seconds;
	tv.tv_usec = 0;

	if ((sck = socket(AF_INET, SOCK_DGRAM, 0)) < 0){ 
		perror("cannot create socket\n"); 
		exit(EXIT_FAILURE);
	}
	
	if (setsockopt(sck, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
    	perror("Error making socket with timeout\n");
		exit(EXIT_FAILURE);
	}
	return sck; 
}

bool bindToSocket(int sck, unsigned short portNum){

	struct sockaddr_in myaddr; 							

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(portNum);

	if (bind(sck, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0){
		perror("bind failed"); 
		//close(sck);
		return false;
	}
	return true;
}


void setBroadcastSocket(int sck, struct sockaddr_in *broadcastAddr,const char* subnetBcast, socklen_t addrlen, unsigned short portNum){
	int broadcastEnable=1;
	int retVal;

	if((retVal = setsockopt(sck, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable))) < 0){
		printf("Error: Could not open set socket to broadcast mode\n");
		close(sck);
		return;
	} 
	memset((char *)broadcastAddr, 0, addrlen);
	broadcastAddr[0].sin_family = AF_INET;
	inet_pton(AF_INET, subnetBcast, &broadcastAddr[0].sin_addr); 			
	broadcastAddr[0].sin_port = htons(portNum); 

}

void setReceiver(struct sockaddr_in *remoteAddr, socklen_t addrlen, const char* remoteIP, unsigned short portNum){
	memset((char *)remoteAddr, 0, addrlen);
	remoteAddr[0].sin_family = AF_INET;
	inet_pton(AF_INET,remoteIP,&remoteAddr[0].sin_addr);
	remoteAddr[0].sin_port = htons(portNum);
}




