/*//////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
WARNING: Encryption might be weak.
////////////////////////////////////////////////////////////////////////////////////////
*///////////////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdio.h>
#include "cryptopp/osrng.h"
#include "cryptopp/modes.h"
#include "cryptopp/aes.h"
#include "cryptopp/sha.h"

#define CHECKSUMLENGTH 32
/*//////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
Function prototypes
////////////////////////////////////////////////////////////////////////////////////////
*///////////////////////////////////////////////////////////////////////////////////////
int		netEncryptSymmetric(char *key, char *data, int len, char *output);
int		netDecryptSymmetric(char *key, char *data, int len, char *output);
void	netCheckSum(char *data, int len, char output[CHECKSUMLENGTH]);
int		netAreCheckSumsEqual(char sum1[CHECKSUMLENGTH], char sum2[CHECKSUMLENGTH]);
/*//////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
Main functions
////////////////////////////////////////////////////////////////////////////////////////
*///////////////////////////////////////////////////////////////////////////////////////
//Symmetric encryption
using namespace CryptoPP;

int formatKey(byte *newKey, char *key){
	int length = strlen(key);
	if(length < AES::MIN_KEYLENGTH){
		for(int i=0; i<AES::MIN_KEYLENGTH; i++) newKey[i] 						= key[i%length];
		return AES::MIN_KEYLENGTH;
	}
	if(length > AES::MAX_KEYLENGTH){
		for(int i=0; i<AES::MAX_KEYLENGTH; i++) newKey[i] 						= 0;
		for(int i=0; i<length; i++) 			newKey[i%AES::MAX_KEYLENGTH] 	^= key[i];
		return AES::MAX_KEYLENGTH;
	}
	else{
		for(int i=0; i<length; i++) 			newKey[i] 						= key[i];
		return length;
	}
}

//warning, make sure the output is big enough
int		netEncryptSymmetric(char *key, char *data, int len, char *output){
	AutoSeededRandomPool rnd;
	byte	newKey[AES::MAX_KEYLENGTH], iv[AES::BLOCKSIZE];
	byte 	plainText[1000], cipherText[1000];
	// Generate a random IV
	rnd.GenerateBlock(iv, AES::BLOCKSIZE);
	//verifyKey
	int 	keyLength = formatKey(newKey, key);
	//
	for(int i=0; i<AES::BLOCKSIZE;	i++) output[i] 						= iv[i];
	for(int i=0; i<len; 			i++) plainText[i] 					= data[i];
	CFB_Mode<AES >::Encryption cfbEncryption(newKey, keyLength, iv);
	cfbEncryption.ProcessData(cipherText, plainText, len);
	for(int i=0; i<len; 			i++) output[i + AES::BLOCKSIZE] 	= cipherText[i];
	//
	return len + AES::BLOCKSIZE;
}

int		netDecryptSymmetric(char *key, char *data, int len, char *output){
	if((len -= AES::BLOCKSIZE) <= 0) return 0;	
	//
	byte	newKey[AES::MAX_KEYLENGTH], iv[AES::BLOCKSIZE];
	byte 	plainText[1000], cipherText[1000];
	int 	keyLength = formatKey(newKey, key);
	for(int i=0; i<AES::BLOCKSIZE;	i++) iv[i] 			= data[i];
	for(int i=0; i<len; 			i++) cipherText[i] 	= data[i + AES::BLOCKSIZE];
	CFB_Mode<AES >::Decryption cfbDecryption(newKey, keyLength, iv);
	cfbDecryption.ProcessData(plainText, cipherText, len);
	for(int i=0; i<len; 			i++) output[i] 		= plainText[i];
	//
	return len;
}

void	netCheckSum(char *data, int len, char output[CHECKSUMLENGTH]){
	byte 	sum[SHA256::DIGESTSIZE], myData[1000];
	for(int i=0; i<len; i++) myData[i] = data[i];
	SHA256().CalculateDigest(sum, myData, len);
	for(int i=0; i<SHA256::DIGESTSIZE;i++) output[i] = (char)sum[i];
}

int		netAreCheckSumsEqual(char sum1[CHECKSUMLENGTH], char sum2[CHECKSUMLENGTH]){
	return 0 == memcmp ( sum1, sum2, SHA256::DIGESTSIZE );
}

