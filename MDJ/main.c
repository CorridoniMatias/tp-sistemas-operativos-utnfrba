#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "headers/MDJlib.h"
#include "headers/FSManager.h"
#include "commons/string.h"

#include "kemmens/StringUtils.h"

void TestBitMapCreateAssignAndFree()
{
	Logger_CreateLog("./DMJ.log", "DMJ", true);
	initGlobals();
	FIFA_Start();

	char* tmp;
	for(int i = 0; i < 20; i++)
	{
		printf("\n\n");
		tmp = StringUtils_Format("/testfile_%d", i);
		FIFA_CreateFile(tmp, 8);
		free(tmp);
	}

	FIFA_ShutDown();

	exit_gracefully_custom((void*)freeGlobals, 0);
}

void TestFIFAReadFile()
{
	Logger_CreateLog("./DMJ.log", "DMJ", true);
	initGlobals();

	//int tam = 15;
	int cop = 0;

	char* cont = FIFA_ReadFile("/scripts/checkpoint.escriptorio", 0, 500, &cop);

	cont = realloc(cont, cop + 1);

	cont[cop] = '\0';

	printf("\nCONTENIDO: '%s' tamanio = %d\n", cont, cop);

	free(cont);

	exit_gracefully_custom((void*)freeGlobals, 0);
}

void TestFIFAWriteFile()
{
	Logger_CreateLog("./DMJ.log", "DMJ", true);
	initGlobals();
	FIFA_Start();

	FIFA_WriteFile("/testfile", 15, 2, "holacomoestaAS");
/*
	int cop = 0;
	char* cont = FIFA_ReadFile("/testfile", 4, 1, &cop);

	cont = realloc(cont, cop + 1);

	cont[cop] = '\0';

	printf("\nCONTENIDO: '%s' tamanio = %d\n", cont, cop);

	free(cont);
*/
	FIFA_ShutDown();
	exit_gracefully_custom((void*)freeGlobals, 0);
}

void TestFIFADeleteFile()
{
	Logger_CreateLog("./DMJ.log", "DMJ", true);
	initGlobals();
	FIFA_Start();

	FIFA_DeleteFile("/testfile");

	FIFA_ShutDown();
	exit_gracefully_custom((void*)freeGlobals, 0);
}

int main(int argc, char **argv)
{

	TestFIFAReadFile();
	//TestBitMapCreateAssignAndFree();
	//TestFIFAWriteFile();
	//TestFIFADeleteFile();

	Logger_CreateLog("./DMJ.log", "DMJ", false);
	initGlobals();
	FIFA_Start();
	startServer();
	ThreadPool_FreeGracefully(threadPool);
	FIFA_ShutDown();
	exit_gracefully_custom((void*)freeGlobals, 0);
}

