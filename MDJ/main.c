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

	for(int i = 0; i < 10; i++)
	{
		FIFA_CreateFile("/testfile", 8);
	}
	FIFA_CreateFile("/testfile", 8);
	FIFA_CreateFile("/testfile1", 8);
	FIFA_CreateFile("/testfile2", 8);
	FIFA_CreateFile("/testfile3", 8);
	FIFA_CreateFile("/testfile4", 8);

	FIFA_ShutDown();

	exit_gracefully_custom((void*)freeGlobals, 0);
}

void TestFIFAReadFile()
{
	Logger_CreateLog("./DMJ.log", "DMJ", true);
	initGlobals();

	//int tam = 15;
	int cop = 0;

	char* cont = FIFA_ReadFile("/scripts/2.script", 0, 5, &cop);

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

	//FIFA_WriteFile("/testfile", 3, 14, "holacomoestaAS");

	int cop = 0;
	char* cont = FIFA_ReadFile("/testfile", 4, 1, &cop);

	cont = realloc(cont, cop + 1);

	cont[cop] = '\0';

	printf("\nCONTENIDO: '%s' tamanio = %d\n", cont, cop);

	free(cont);

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

	//TestFIFAReadFile();
	TestBitMapCreateAssignAndFree();
	//TestFIFAWriteFile();
	//TestFIFADeleteFile();

	Logger_CreateLog("./DMJ.log", "DMJ", true);

	startServer();
	ThreadPool_FreeGracefully(threadPool);
	exit_gracefully_custom((void*)freeGlobals, 0);
}

