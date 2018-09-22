#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "headers/MDJlib.h"
#include "headers/FSManager.h"

#include "kemmens/StringUtils.h"

int main(int argc, char **argv)
{

	printf("%f", ((9/(float)4) - 1) * 4);

	return 0;
	Logger_CreateLog("./DMJ.log", "DMJ", true);
	Logger_Log(LOG_INFO, "Proceso DMJ iniciado...");
	initGlobals();

	FIFA_Init();

	FIFA_ReadBitmap();

	bool b1 = FIFA_IsBlockUsed(0);
	bool b2 = FIFA_IsBlockUsed(1);
	bool b3 = FIFA_IsBlockUsed(2);
	bool b4 = FIFA_IsBlockUsed(3);

	printf("b1: %d \t b2: %d \t b3: %d \t b4:%d\n", b1, b2, b3, b4);

	FIFA_SetUsedBlock(1);
	FIFA_SetUsedBlock(3);

	b1 = FIFA_IsBlockUsed(0);
	b2 = FIFA_IsBlockUsed(1);
	b3 = FIFA_IsBlockUsed(2);
	b4 = FIFA_IsBlockUsed(3);

	 printf("b1: %d \t b2: %d \t b3: %d \t b4:%d\n", b1, b2, b3, b4);

	FIFA_FlushBitmap();

	FIFA_FreeBitmap();

	//startServer();
	//ThreadPool_FreeGracefully(threadPool);
	exit_gracefully_custom((void*)freeGlobals, 0);
}

