#include "UDP.h"

int createUDPSocket(){
	int sck;

	if ((sck = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
		perror("cannot create socket\n"); 
		exit(EXIT_FAILURE);
	}
	return sck; 
}

//Creates an UDP socket with a specified receive timeout
int createUDPTimeoutSocket(long int seconds){
	int sck;
	struct timeval tv;

	tv.tv_sec = seconds;
	tv.tv_usec = 0;

	if ((sck = socket(AF_INET, SOCK_DGRAM, 0)) < 0){ 
		perror("cannot create socket\n"); 
		exit(EXIT_FAILURE);
	}
	
	if (setsockopt(sck, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){									//Set the receive timeout
    	perror("Error making socket with timeout\n");
		exit(EXIT_FAILURE);
	}
	return sck; 
}

//Tries to bind the server to a specified port
bool bindToSocket(int sck, unsigned short portNum){

	struct sockaddr_in myaddr; 							

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);															//Allows the program to work without knowing the IP address of the machine it was running on
	myaddr.sin_port = htons(portNum);																	//Specifying the port which is going to be listened to

	if (bind(sck, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0){
		perror("bind failed"); 
		return false;
	}
	return true;
}

//Tries to set the socket in Broadcast mode; it will bradcast to the local subnet
void setBroadcastSocket(int sck, struct sockaddr_in *broadcastAddr,const char* subnetBcast, socklen_t addrlen, unsigned short portNum){
	int broadcastEnable=1;
	int retVal;

	if((retVal = setsockopt(sck, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable))) < 0){
		printf("Error: Could not open set socket to broadcast mode\n");
		close(sck);
		return;
	} 
	memset((char *)broadcastAddr, 0, addrlen);																//Filling the socket address structure with subnet mask and used port
	broadcastAddr[0].sin_family = AF_INET;
	inet_pton(AF_INET, subnetBcast, &broadcastAddr[0].sin_addr); 			
	broadcastAddr[0].sin_port = htons(portNum); 

}



