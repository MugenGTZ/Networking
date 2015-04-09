/*//////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
This network module is a simplification of the UDP and TCP network protocols.
	It allow us to communicate with other clients without the need to use their IP address,
	instead we send packages using their client's netcard(indentifying tag) number.

This network module should do the following
	1. Create easy connections to other addresses (within the same computer or in a network)
	2. Communicate securely (encrypt data) (extra*)
	3. Make integrity checks (Netowrk errors or tampering)
////////////////////////////////////////////////////////////////////////////////////////
*///////////////////////////////////////////////////////////////////////////////////////

#ifndef _NETWORK_H_
#define _NETWORK_H_
	#include <stdint.h>
	#include <unistd.h>
	#define NETCARDBROADCAST 	0
	typedef enum {NE_NO_ERROR, NE_TIMEOUT, NE_NETCARD_NOT_FOUND, NE_SENDING_FAILED, NE_DATA_CORRUPTION} nErr;
	typedef uint64_t chan;
	typedef uint64_t netcard;

	//
	netcard	getMyNetCard();

	//Initialize network protocol
	void 	initNetwork();
	void	setNetworkPassword(char *pass);

	//Connectionless network protocol
	void 	dataServerCallBack(void (*fun)(netcard num, char *data, int32_t len));
	void	sendData(netcard num, char *data, int32_t len);
	void	sendBroadcast(char *data, int32_t len);

	//Connection oriented network protocol
	void	chanServerCallBack(void (*fun)(chan extChan));
	chan	createChannel(netcard num);
	void	acceptChannel(chan extChan);
	void	closeChannel(chan extChan);

	nErr	chanSend(chan extChan, char *data, ssize_t len);
	nErr	chanRecv(chan extChan, char *data, ssize_t *len, size_t maxLen, long int sec_timeout);

	//error Display
	void	displayErr(nErr e);
#endif
