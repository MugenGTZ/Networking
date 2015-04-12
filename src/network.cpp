 #include "network.h"
#include "stdio.h"
#include "rand64.h"
#include "UDP.h"
#include "TCP.h"
#include "netCrypt.h"

#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <pthread.h>
#include <map>

typedef struct _peerInfo{
	char 	IP[NI_MAXHOST];
	int32_t TCPport;
}peerInfo;

netcard myNum;
std::map<netcard, peerInfo> netcardMap;
void (*dataServerCallBackFun)(netcard, char *, int32_t);
void (*chanServerCallBackFun)(chan extChan);
#define NPORTS				5

int32_t 	TCPPORT 		= 2050;
const char 	TCP_PORT[] 		= "2052";
static char _password[100] 	= "root";

unsigned short ports[] = {49845,46532,31940,61361,19553,47144,33427,51413,59243,3715,25990,37660,11669,33685,14060,52152,35630,62479,33372,19254,8237,15836,57984,54667,12277,52407,31734,2539};

int getComputerIPV4(struct in_addr *addr);
void printIfAddr(struct in_addr *address);
void makeAddrBroadcastAddr(struct in_addr *addrPtr);

void* 	listenUDPBroadcast(void* nothing);
void* 	listenChanBroadcast(void* nothing);
int 	getServerSocket();

struct in_addr myAddress;
//Initialize network protocol
void 	initNetwork(){
	initRand64();
	static int networkHasInitialized;
	if(networkHasInitialized) return;
	else networkHasInitialized = 1;

	//Get my IP
	getComputerIPV4(&myAddress);
	makeAddrBroadcastAddr(&myAddress);
	printIfAddr(&myAddress);

	//The possibility of collision of 100 clients is less than 3.6E-15
	myNum = (netcard) rand64();
	printf("My netcard is:%llu\n",(long long unsigned int)myNum);

	//init TCP port
	getServerSocket();

	pthread_t threadDataListen, threadChanListen;
	if(pthread_create(&threadDataListen, NULL, listenUDPBroadcast, NULL)) {
		fprintf(stderr, "Error creating thread\n");
		return;
	}
	if(pthread_create(&threadChanListen, NULL, listenChanBroadcast, NULL)) {
		fprintf(stderr, "Error creating thread\n");
		return;
	}
	//TODO
}

void	setNetworkPassword(char *pass){
	strcpy(_password, pass);
}

netcard	getMyNetCard(){
	return myNum;
}
////////////////////////////////////////////////////////////////////////////////////////
//Connectionless network protocol
////////////////////////////////////////////////////////////////////////////////////////
void 	dataServerCallBack(void (*fun)(netcard num, char *data, int32_t len)){
	dataServerCallBackFun = fun;
}
//Make a package as follows encrypt([org. netcard, dest. netcard, len, checksum1, data, checksum2])
enum SENDTYPE{ST_NORMAL, ST_TCPADDR};
void	sendDataAdv(netcard num, SENDTYPE type, char *data, int32_t len);

void	sendData(netcard num, char *data, int32_t len){	
	sendDataAdv(num, ST_NORMAL, data, len);
}

void	sendDataAdv(netcard num, SENDTYPE type, char *data, int32_t len){	
	int socket;
	int _type = (int32_t)type;
	int32_t _tcpport = (int32_t) TCPPORT;
	struct sockaddr_in bcastAddr;
	socklen_t addrlen = sizeof(bcastAddr);
	char subnetAddr[INET_ADDRSTRLEN];													//put broadcast address of subnet
    inet_ntop(AF_INET, &myAddress, subnetAddr, INET_ADDRSTRLEN);

	char checksum1[CHECKSUMLENGTH], checksum2[CHECKSUMLENGTH];
										
	int sizeofPack = len+2*sizeof(netcard)+3*sizeof(int32_t)+2*CHECKSUMLENGTH;
	char *pack 		= (char *) malloc(sizeofPack);
	char *packCrypt = (char *) malloc(sizeofPack+64);
	if(NULL == pack) {printf("Malloc error"); return;}
	int ptr = 0;
	memcpy(&pack[ptr],&myNum, sizeof(netcard));		ptr += sizeof(netcard);
	memcpy(&pack[ptr],&num, sizeof(netcard));		ptr += sizeof(netcard);
	memcpy(&pack[ptr],&_type, sizeof(int32_t));		ptr += sizeof(int32_t);
	memcpy(&pack[ptr],&_tcpport, sizeof(int32_t));	ptr += sizeof(int32_t);
	memcpy(&pack[ptr],&len, sizeof(int32_t));		ptr += sizeof(int32_t);	

	netCheckSum(pack, ptr, checksum1);
	netCheckSum(data, len, checksum2);
	memcpy(&pack[ptr],checksum1, CHECKSUMLENGTH);	ptr += CHECKSUMLENGTH;
	memcpy(&pack[ptr],data, len);					ptr += len;
	memcpy(&pack[ptr],checksum2, CHECKSUMLENGTH);	ptr += CHECKSUMLENGTH;

	sizeofPack = netEncryptSymmetric(_password, pack, sizeofPack, packCrypt);
	
	for(int i=0; i<NPORTS; i++){
		socket = createUDPSocket();
		setBroadcastSocket(socket, &bcastAddr, subnetAddr, addrlen, ports[i]);
		sendto(socket, packCrypt, sizeofPack, 0, (struct sockaddr *)&bcastAddr, addrlen);
		close(socket);
	}
	free(pack);
	free(packCrypt);
}

void	sendBroadcast(char *data, int32_t len){
	sendData(NETCARDBROADCAST, data, len);
}

////////////////////////////////////////////////////////////////////////////////////////
//Connection oriented network protocol
////////////////////////////////////////////////////////////////////////////////////////
void	chanServerCallBack(void (*fun)(chan extChan)){
	chanServerCallBackFun = fun;
}
chan	createChannel(netcard num){
	char port[50];
	if(netcardMap.find(num) != netcardMap.end()){
		peerInfo temp = netcardMap[num];
		sprintf(port, "%d",temp.TCPport);
		printf("Trying to connect to :%s at port :%s\n",temp.IP,port);
		//return (chan) connect2TCPServer(temp.IP, port);					//TODO: check 10 second value FS changed this on Monday
		return (chan) connect2TCPServerTimeout(temp.IP, port, 5);
	}
	return (chan)-1;
}
void	acceptChannel(chan extChan){
	
	//TODO
}
void	closeChannel(chan extChan){
	close((int)extChan);
}

nErr	chanSend(chan extChan, char *data, ssize_t len){
	//TODO:encrypt
	char dataIn[2048], dataOut[2048];
	netCheckSum(data, len, dataIn);
	memcpy(&dataIn[CHECKSUMLENGTH], data, len);
	len = netEncryptSymmetric(_password, dataIn, len + CHECKSUMLENGTH, dataOut);
	
	ssize_t sentBytes = send_TCP_msg(extChan, dataOut, len);
	if(sentBytes == -1) return NE_SENDING_FAILED;
	else 				return NE_NO_ERROR;
}
nErr	chanRecv(chan extChan, char *data, ssize_t *len, size_t maxLen, long int sec_timeout){
	char dataOut[2048], dataIn[2048], checkSum[CHECKSUMLENGTH];
	
	len[0] = rcv_TCP_msg(extChan, dataIn, maxLen + 128, sec_timeout);
	if((len[0] == -2) || (len[0] == 0)) return NE_NETCARD_NOT_FOUND;
	if(-1 != len[0]){
		len[0] = netDecryptSymmetric(_password, dataIn, (int)*len, dataOut);
		if((len[0] -= CHECKSUMLENGTH) <= 0) {printf("failed because length < 32\n"); return NE_DATA_CORRUPTION;}
		
		netCheckSum(&dataOut[CHECKSUMLENGTH], len[0], checkSum);
		if(!netAreCheckSumsEqual(checkSum, dataOut)) {printf("failure: incorrect checksum\n"); return NE_DATA_CORRUPTION;}

		memcpy(data, &dataOut[CHECKSUMLENGTH], len[0]);
		return NE_NO_ERROR;
	}
	return NE_TIMEOUT;
}

////////////////////////////////////////////////////////////////////////////////////////
//error Display
////////////////////////////////////////////////////////////////////////////////////////
void	displayErr(nErr e){
	switch(e){
		case NE_NO_ERROR:			printf("No error!\n");							break;
		case NE_TIMEOUT:			printf("Connection time out!\n");				break;
		case NE_NETCARD_NOT_FOUND:	printf("Netcard not found!\n");					break;
		case NE_SENDING_FAILED:		printf("Sending on TCP socket failed!\n");		break;
		case NE_DATA_CORRUPTION:	printf("The data is corrupted!\n");				break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//Helper Functions
////////////////////////////////////////////////////////////////////////////////////////
int getComputerIPV4 (struct in_addr *addr) {
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;

	struct sockaddr_in	local;
	struct in_addr		*test;
	inet_pton(AF_INET, "127.0.0.1", &(local.sin_addr));

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family == AF_INET) { // is IP4 valid?
            test=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			if(0 != memcmp(test, &(local.sin_addr), sizeof(struct in_addr))){
				memcpy(addr, test, sizeof(struct ifaddrs));
				break;
			}
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return 0;
}

int decodeAddressIPV4(struct in_addr *addrPtr){
	int output = 0;
	char *addr = (char *) &output;
	int tmp[4];
	char addressBuffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, addrPtr, addressBuffer, INET_ADDRSTRLEN);
	sscanf(addressBuffer, "%d.%d.%d,%d",&tmp[3],&tmp[2],&tmp[1],&tmp[0]);
	addr[0] = (unsigned char) tmp[0];
	addr[1] = (unsigned char) tmp[1];
	addr[2] = (unsigned char) tmp[2];
	addr[3] = (unsigned char) tmp[3];
	return output;
}

void encodeAddressIPV4(struct in_addr *addrPtr,int num){
	unsigned char *addr = (unsigned char *) &num;
	char addressBuffer[INET_ADDRSTRLEN];
	sprintf(addressBuffer, "%d.%d.%d.255", addr[3], addr[2], addr[1]);
	inet_pton(AF_INET, addressBuffer, addrPtr);
}

void makeAddrBroadcastAddr(struct in_addr *addrPtr){
	int numAddr = decodeAddressIPV4(addrPtr);
	encodeAddressIPV4(addrPtr,numAddr);
}

void printIfAddr(struct in_addr *address){
    char addressBuffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, address, addressBuffer, INET_ADDRSTRLEN);
    printf("IP Address: %s\n", addressBuffer);
}

void broadcastOverUDP(void* buffer, int length){
	printf("Snt Mesage:%s\n",(char*)buffer);
	int socket;
	struct sockaddr_in bcastAddr;
	char addressBuffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &bcastAddr, addressBuffer, INET_ADDRSTRLEN);
	
	socket = createUDPSocket();
	setBroadcastSocket(socket, &bcastAddr, addressBuffer, INET_ADDRSTRLEN, 40000);
	
	sendto(socket, buffer, length, 0, (struct sockaddr *)&bcastAddr, INET_ADDRSTRLEN);
	close(socket);
	return;
}

void* listenUDPBroadcastThread(void* param){
	void **mem_ptr = (void**)param;
	if(NULL != dataServerCallBackFun) dataServerCallBackFun(*((netcard*)mem_ptr[0]), (char*)mem_ptr[1], *((int32_t*)mem_ptr[2]));
	free(mem_ptr[0]);
	free(mem_ptr[1]);
	free(mem_ptr[2]);
	free(mem_ptr);
	return 0;
}

void* listenUDPBroadcast(void* nothing){
	int socket;
	char buffer[1000];
	char bufferCrypt[1000];

	socket = createUDPSocket();
	int i = 0;
	bool binded = false;
	while((i < NPORTS) && !(binded=bindToSocket(socket, ports[i++])));
	if(!binded){printf("Could not bind to any port!\n");exit(EXIT_FAILURE);}

	while(1){
		memset(buffer, 0, 1000);
		memset(bufferCrypt, 0, 1000);
		struct sockaddr_in remaddr;
		socklen_t addrlen = sizeof(remaddr);//INET_ADDRSTRLEN;
		int rclen = recvfrom(socket, bufferCrypt, 1000, 0, (struct sockaddr *)&remaddr, &addrlen);

		//TODO: MAP
       char host[NI_MAXHOST], service[NI_MAXSERV];

       getnameinfo((struct sockaddr *) &remaddr,addrlen, host, NI_MAXHOST,service, NI_MAXSERV, NI_NUMERICSERV);

		rclen = netDecryptSymmetric(_password, bufferCrypt, rclen, buffer);
	
		netcard origin, dest;
		char checksum1[CHECKSUMLENGTH], checksum2[CHECKSUMLENGTH],csVerify[CHECKSUMLENGTH];
		int32_t len;

		int ptr = 0;
		int32_t type, tcpport;
		memcpy(&origin, 	&buffer[ptr], sizeof(netcard));		ptr += sizeof(netcard);
		memcpy(&dest, 		&buffer[ptr], sizeof(netcard));		ptr += sizeof(netcard);
		memcpy(&type, 		&buffer[ptr], sizeof(int32_t));		ptr += sizeof(int32_t);
		memcpy(&tcpport, 	&buffer[ptr], sizeof(int32_t));		ptr += sizeof(int32_t);
		memcpy(&len, 		&buffer[ptr], sizeof(int32_t));		ptr += sizeof(int32_t);
		netCheckSum(buffer, ptr, csVerify);
		memcpy(checksum1,	&buffer[ptr], CHECKSUMLENGTH);		ptr += CHECKSUMLENGTH;
		//printf("%d==%d,\n",(int)checksum1[0],(int)csVerify[0]);
		if(netAreCheckSumsEqual(csVerify, checksum1)){
			char *data = (char *) malloc(len);

			memcpy(data, 		&buffer[ptr], len);					ptr += len;
			memcpy(checksum2,	&buffer[ptr], CHECKSUMLENGTH);		ptr += CHECKSUMLENGTH;

			netCheckSum(data, len, csVerify);
			if(!netAreCheckSumsEqual(csVerify, checksum2)){
				printf("Msg is incorrect!\n");
				continue;
			}

			//printf("card = %d == %d ?\n",dest,myNum);
			if((dest == NETCARDBROADCAST && origin != myNum) || dest == myNum){
				peerInfo temp;
				temp.TCPport = tcpport;
				memcpy(temp.IP, host, NI_MAXHOST);
				if(netcardMap.find(origin) == netcardMap.end()) netcardMap[origin] = temp;			//does not exist? add new entry

				void** 	mem_ptr;
				pthread_t thread;
				switch((SENDTYPE)type){
					case ST_NORMAL:
						mem_ptr		= (void**)malloc(sizeof(void*)*3);
						mem_ptr[0] 	= malloc(sizeof(netcard));
						mem_ptr[1] 	= malloc(len);
						mem_ptr[2]	= malloc(sizeof(int32_t));
						memcpy(mem_ptr[0], &origin, sizeof(netcard));
						memcpy(mem_ptr[1], data, len);
						memcpy(mem_ptr[2], &len, sizeof(int32_t));
						pthread_create(&thread, NULL, listenUDPBroadcastThread, mem_ptr);
						break;
					case ST_TCPADDR: 
						break;
				}
				//printf("enters\n");
			}
			else{ //printf("does not enter\n");
			}
			
			//printf("Rcv Mesage:");
			//printf("Received pack from:%llu, for:%llu with:%s\n",(long long unsigned int)origin, (long long unsigned int)dest, data);

			free(data);
		}
		else{
			printf("Msg is incorrect!\n");
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
#define BUFSIZE 1024
typedef struct sockaddr_in sockaddr_info;	

typedef struct _clientInfo{									// I put this structure as a parameter for thread func
					int socked_fd;
					char* host_addr;
				}clientInfo;

void ObtainServerAddr(sockaddr_info *serveraddr, int port){
	bzero((char *) serveraddr, sizeof(sockaddr_info)); 		
	serveraddr[0].sin_family = AF_INET;
	serveraddr[0].sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr[0].sin_port = htons((unsigned short)port);
}

void createServerSocket(int *socked_fd){
	for(int i = 0; i < 20; i++){
		TCPPORT = rand() & 0x00007FFF;
		sockaddr_info serveraddr; 

		socked_fd[0] = socket(AF_INET, SOCK_STREAM, 0);
		if (socked_fd[0] < 0) perror("ERROR opening socket");
	
		int optval = 1;
		setsockopt(socked_fd[0], SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
	
		ObtainServerAddr(&serveraddr, TCPPORT);

		if (bind(socked_fd[0], (struct sockaddr *) &serveraddr, sizeof(sockaddr_info)) < 0){
			perror("ERROR on binding");
		}
		else{
			printf("MY TCP PORT: %d\n",TCPPORT);
			break;
		}
	}
}

void* thread_func(void *clientID){
	clientInfo *client_info = (clientInfo*) clientID;
	chanServerCallBackFun((chan) client_info -> socked_fd);
	return 0;
}

static int _parentfd = -1;
int getServerSocket(){
	if(_parentfd < 0) createServerSocket(&_parentfd);
	return _parentfd;
}

void* listenChanBroadcast(void* nothing){
	int parentfd = getServerSocket(); 
	int childfd; 											// child socket
	sockaddr_info clientaddr; 								// client addr
	clientInfo connectedClient;
  															// listen: make this socket ready to accept connection requests 
  	if (listen(parentfd, 5) < 0) 							// allow 5 requests to queue up
    	perror("ERROR on listen");
	
	printf("Server has started and is listening to port TCPPORT.\n");

	socklen_t clientlen = sizeof(sockaddr_info);
	while (1){
		if ((childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen))< 0) perror("ERROR on accept");

		char host[NI_MAXHOST], service[NI_MAXSERV];
		int s = getnameinfo((struct sockaddr *) &clientaddr, clientlen, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);

		if (s == 0)
			printf("Connected to: %s:%s\n", host, service);
		else{
			printf("Error with getnameinfo!\n");
			exit(EXIT_FAILURE);
		}
		//serve the client
		pthread_t thread;

		connectedClient.socked_fd = childfd;
		connectedClient.host_addr = host;
		pthread_create(&thread, NULL, thread_func, (void*) &connectedClient);
	}
}


























