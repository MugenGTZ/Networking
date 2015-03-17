#include <stdio.h>
#include "../src/network.h"
#include <unistd.h>
#include <string.h>

void networkConnectionlessCallBack(netcard num, char *data, char len){
	char _data[256];
	ssize_t _len;
	nErr error;

	printf("Received pack from:%llu with:%s\n",(long long unsigned int)num, data);
	chan Channel = createChannel(num);
	if(Channel == (chan)-1) {printf("ERROR TCP\n");return;}	
	displayErr(chanSend(Channel, (char*)"Can you hear me?", strlen("Can you hear me?") + 1));

	if(NE_NO_ERROR != (error = chanRecv(Channel, _data, &_len, 256, 5))){ displayErr(error); closeChannel(Channel); return; }
	_data[_len] = 0;	
	printf("Received from %llu: %s\n", (unsigned long long)num, _data);

	closeChannel(Channel);
}

void networkConnectionOrientedCallBack(chan extChan){
	char data[256];
	ssize_t len;
	nErr error;

	if(NE_NO_ERROR != (error = chanRecv(extChan, data, &len, 256, 5))){ displayErr(error); closeChannel(extChan); return; }
	data[len] = 0;	
	printf("Received: %s\n", data);

	displayErr(chanSend(extChan, (char*)"Ok, got you!", strlen("Ok, got you!")+1));
	closeChannel(extChan);
}

int main(){
	//setup
	initNetwork();
	chanServerCallBack(networkConnectionOrientedCallBack);
	dataServerCallBack(networkConnectionlessCallBack);

	

	//connectionless
	while(1){
		sendData(NETCARDBROADCAST, (char*)"Hello World!", 13);
		printf("UDP bcast\n");
		usleep(1000000);
	}
}
