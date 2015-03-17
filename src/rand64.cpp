#include <time.h>
#include <stdlib.h>
#include "rand64.h"

void 		initRand64(){
	static int randHasInitialized;
	if(randHasInitialized) return;
	else randHasInitialized = 1;
	srand(time(NULL));
}

uint64_t	rand64(){
	uint64_t	n16_01 = rand() & 0x000000000000FFFF;
	uint64_t	n16_02 = rand() & 0x000000000000FFFF;
	uint64_t	n16_03 = rand() & 0x000000000000FFFF;
	uint64_t	n16_04 = rand() & 0x000000000000FFFF;
	n16_01 	<<= 0;
	n16_02 	<<= 16;
	n16_03 	<<= 32;
	n16_04 	<<= 48;
	return n16_01 | n16_02 | n16_03 | n16_04;
}
