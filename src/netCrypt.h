/*//////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
WARNING: Make sure that the library has strong Encryption!
////////////////////////////////////////////////////////////////////////////////////////
*///////////////////////////////////////////////////////////////////////////////////////

#ifndef _NETCRYPT_H_
#define _NETCRYPT_H_
	#define CHECKSUMLENGTH 32
	//Symmetric encryption
	void	netEncryptSymmetric(char *key, char *data, int len, char *output);
	void	netDecryptSymmetric(char *key, char *data, int len, char *output);
	//Check sum
	void	netCheckSum(char *data, int len, char output[CHECKSUMLENGTH]);
	int		netAreCheckSumsEqual(char sum1[CHECKSUMLENGTH], char sum2[CHECKSUMLENGTH]);
#endif